using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace OpenZiti.Debugging {
    public class LoggingHandler : DelegatingHandler {

        public bool DoLogging { get; set; }

        public LoggingHandler(HttpMessageHandler innerHandler)
            : base(innerHandler) {
        }

        protected override async Task<HttpResponseMessage> SendAsync(HttpRequestMessage request, CancellationToken cancellationToken) {
            if (DoLogging) {
                Console.WriteLine("Request:");
                Console.WriteLine(request.ToString());
                if (request.Content != null) {
                    Console.WriteLine(await request.Content.ReadAsStringAsync());
                }
                Console.WriteLine();
            }
            HttpResponseMessage response = await base.SendAsync(request, cancellationToken);
            if (DoLogging) {
                Console.WriteLine("Response:");
                Console.WriteLine(response.ToString());
                if (response.Content != null) {
                    Console.WriteLine(await response.Content.ReadAsStringAsync());
                }
                Console.WriteLine();
                Console.WriteLine("===============================================================================================");
                Console.WriteLine();
            }
            return response;
        }
    }
}