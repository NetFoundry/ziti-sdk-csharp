﻿/*
Copyright NetFoundry Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using OpenZiti.Native;

namespace OpenZiti
{
    /// <summary>
    /// This class encapsulates a native Ziti service and provides the basic methods
    /// necessary to use a Ziti-based service.
    /// </summary>
    public class ZitiService
    {
        private string name = null;

        /// <summary>
        /// The Name of the <see cref="ZitiService"/>
        /// </summary>
        public string Name {
            get {
                if (name == null) {
                    name = nativeService.name;
                }
                return name;
            }
        }

        public string Id {
	        get { return nativeService.id; }
        }

        public Dictionary<String, PostureQuerySet> PostureQueryMap {
            get; internal set;
        }

        public ZitiIdentity Identity { get; internal set; }

        internal readonly byte[] NO_DATA = new byte[0];

        internal IntPtr nativeServicePointer;
        private ziti_service nativeService;
        private ZitiContext zitiContext;
        private ZitiConnection conn;
        private OnZitiConnected onConnected;
        private OnZitiDataReceived onData;
        private OnZitiListening listenCallback;
        private OnZitiClientConnected onClientConnected;

        internal ZitiService(ZitiIdentity id, ZitiContext context, IntPtr ziti_service)
        {
            this.Identity = id;
            zitiContext = context;
            this.nativeServicePointer = ziti_service;
            nativeService = Marshal.PtrToStructure<ziti_service>(ziti_service);
            this.PostureQueryMap = getPostureQueryMap(nativeService);
        }

        /// <summary>
        /// This is the function used to initiate the chain of callbacks that will allow the user of the
        /// sdk to actually establish connectivity.
        /// 
        /// Two callbacks must be provided in this function which will allows the callee to control the
        /// data flow. <see cref="onConnected"/> will be called first and only once. After which <see cref="onData"/>
        /// may be called repeatedly.
        /// </summary>
        /// <param name="onConnected">A callback which is called after the Dial function completes. This callback
        /// will contain the result of the invocation. It is highly recommend to verify the result of the function.</param>
        /// <param name="onData">A callback called whenever data arrives on the connection</param>
        public void Dial(OnZitiConnected onConnected, OnZitiDataReceived onData)
        {
            this.onConnected = onConnected;
            this.onData = onData;
            ZitiConnection conn = new ZitiConnection(this, zitiContext, "this is context in my connection");
            this.conn = conn;
            Native.API.ziti_dial(conn.nativeConnection, Name, conn_cb, data_cb);
        }

        /// <summary>
        /// Used to indicate that this program should listen for and accept connections from other identities.
        /// 
        /// Any call to <see cref="Listen"/> must be configured to host the service using the Ziti Controller.
        /// 
        /// Two callbacks are required to be supplied. <see cref="listenCallback"/> is called after the Listen
        /// function completes will contain the result of the invocation. It is highly recommend to verify the 
        /// result of the function. <see cref="onClient"/> will be called any time another identity tries to 
        /// <see cref="Dial(OnZitiConnected, OnZitiDataReceived)"/> this service.
        /// </summary>
        /// <param name="listenCallback"></param>
        /// <param name="onClient"></param>
        public void Listen(OnZitiListening listenCallback, OnZitiClientConnected onClient)
        {
            this.listenCallback = listenCallback;
            onClientConnected = onClient;

            ZitiConnection conn = new ZitiConnection(this, zitiContext, "this is context in my connection");
            this.conn = conn;
            Native.API.ziti_listen(conn.nativeConnection, Name, native_listen_cb, native_on_client_cb);
        }

        private void conn_cb(IntPtr ziti_connection, int status)
        {
            onConnected(conn, (ZitiStatus)status);
        }

        private int data_cb(IntPtr nativeConn, IntPtr rawData, int len)
        {
            if (len < 0)
            {
                onData(conn, (ZitiStatus)len, NO_DATA);
            }
            else
            {
                byte[] data = new byte[len];
                Marshal.Copy(rawData, data, 0, data.Length);
                onData(conn, ZitiStatus.OK, data);
            }
            return len;
        }

        private void native_listen_cb(IntPtr ziti_connection, int status)
        {
            ZitiConnection conn = new ZitiConnection(this, zitiContext, "this is context in my connection");
            this.conn = conn;
            listenCallback(conn, (ZitiStatus)status);
        }

        private void native_on_client_cb(IntPtr ziti_connection_server, IntPtr ziti_connection_client, int status)
        {
            ZitiConnection svr = new ZitiConnection(this, zitiContext, "this is context in my connection");
            svr.nativeConnection = ziti_connection_server;
            ZitiConnection client = new ZitiConnection(this, zitiContext, "this is context in my connection");
            client.nativeConnection = ziti_connection_client;
            onClientConnected(svr, client, (ZitiStatus)status);
        }

        public string GetConfiguration(string configName) {
            return API.GetConfiguration(this, configName);
        }

        private Dictionary<string, PostureQuerySet> getPostureQueryMap(ziti_service nativeService) {

            Dictionary<string, PostureQuerySet> postureQMap = new Dictionary<string, PostureQuerySet>();
            if (nativeService.posture_query_map != IntPtr.Zero) {
                model_map_impl impl = Marshal.PtrToStructure<model_map_impl>(nativeService.posture_query_map);
                int sizeOfPointer = Marshal.SizeOf(typeof(IntPtr));
                int sizeOfPostuerQueryMap = impl.size;
                Console.WriteLine("Posture Query map size : " + sizeOfPostuerQueryMap);
                IntPtr entriesArr = impl.entries; // loop through entries
                IntPtr currentEntryArrLoc;
                for (int i = 0; i < sizeOfPostuerQueryMap; i++) {
                    if (sizeOfPostuerQueryMap > 1) {
                        entriesArr = IntPtr.Add(entriesArr, sizeOfPointer);
                        currentEntryArrLoc = Marshal.ReadIntPtr(entriesArr);
                        if (currentEntryArrLoc == IntPtr.Zero) {
                            break;
                        }
                    } else {
                        currentEntryArrLoc = entriesArr;
                    }

                    model_map_entry entry = Marshal.PtrToStructure<model_map_entry>(currentEntryArrLoc);
                    string policyId = Marshal.PtrToStringUTF8(entry.key);
                    Console.WriteLine("Posture Query Policy Id : " + policyId);
                    posture_query_set pqs = Marshal.PtrToStructure<posture_query_set>(entry.value);
                    Console.WriteLine("PQS policy id : " + pqs.policy_id);
                    IntPtr pqArr = pqs.posture_queries;
                    IntPtr currentPQArrLoc = pqArr;
                    PostureQuerySet pqSet = new PostureQuerySet();
                    pqSet.PolicyId = pqs.policy_id;
                    pqSet.IsPassing = pqs.is_passing;
                    pqSet.PolicyType = pqs.policy_type;

                    List<PostureQuery> postureQueriesList = new List<PostureQuery>();
                    while ((currentPQArrLoc = Marshal.ReadIntPtr(pqArr)) != IntPtr.Zero) {
                        posture_query pq = Marshal.PtrToStructure<posture_query>(currentPQArrLoc);
                        PostureQuery postureQuery = new PostureQuery();

                        Console.WriteLine("pq query type : " + pq.query_type);
                        postureQuery.QueryType = pq.query_type;
                        postureQuery.Id = pq.id;
                        postureQuery.IsPassing = pq.is_passing; 
                        if (pq.process != IntPtr.Zero) {
                            ZitiProcess ziti_process = new ZitiProcess();
                            ziti_process process = Marshal.PtrToStructure<ziti_process>(pq.process);
                            ziti_process.Path = process.path;
                            postureQuery.Process = ziti_process;
                            Console.WriteLine("process path : " + process.path);
                        }
                        postureQueriesList.Add(postureQuery);
                        pqArr = IntPtr.Add(pqArr, sizeOfPointer);
                    }
                    pqSet.PostureQueries = postureQueriesList.ToArray();
                    postureQMap.Add(policyId, pqSet);

                }

            }
            return postureQMap;
        }
    }

    public struct PostureQuerySet {
        public string PolicyId;
        public bool IsPassing;
        public string PolicyType;
        public PostureQuery[] PostureQueries;
    }
    public struct PostureQuery {
        public string Id;
        public bool IsPassing;
        public string QueryType;
        public ZitiProcess Process;
        public int Timeout;
    }

    public struct ZitiProcess {
        public string Path;
    }
}
