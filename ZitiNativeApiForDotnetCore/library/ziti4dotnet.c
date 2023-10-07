#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#include <ziti/ziti.h>
#include <uv.h>
#include "ziti4dotnet.h"
#include "ziti/zitilib.h"

#if _WIN32
#define strncasecmp _strnicmp
#define strdup _strdup
#endif

#define MAXBUFFERLEN 4096 * 4096

int z4d_ziti_close(ziti_connection con) {
    return 0;
    //return ziti_close(&con);
}

int z4d_uv_run(void* loop) {
    ZITI_LOG(DEBUG, "running loop with address: %p", loop);
    return uv_run(loop, UV_RUN_DEFAULT);
}

const char* ALL_CONFIG_TYPES[] = {
        "all",
        NULL
};
extern const char** z4d_all_config_types() {
    return ALL_CONFIG_TYPES;
}

uv_loop_t* z4d_default_loop()
{
    return uv_default_loop();
}

void* z4d_registerUVTimer(uv_loop_t * loop, uv_timer_cb timer_cb, uint64_t delay, uint64_t iterations) {
    uv_timer_t * uvt = calloc(1, sizeof(uv_timer_t));
    uv_timer_init(loop, uvt);
    uv_timer_start(uvt, timer_cb, iterations, delay);
    return uvt;
}

void* z4d_stop_uv_timer(uv_timer_t* t) {
    uv_timer_stop(t);
}

void* z4d_new_loop() {
    return uv_loop_new();
}

int z4d_event_type_from_pointer(const ziti_event_t *event) {
    return event->type;
}

ziti_service* z4d_service_array_get(ziti_service_array arr, int idx) {
    return arr ? arr[idx] : NULL;
}

char** z4d_make_char_array(int size) {
    return calloc(sizeof(char*), size + 1);
}

void z4d_set_char_at(char **arr, char *val, int idx) {
    char* dupe = strdup(val);
    arr[idx] = dupe;
}

void z4d_free_char_array(char **a, int size) {
    int i;
    for (i = 0; i < size; i++) {
        free(a[i]);
    }
    free(a);
}

typedef int (*printer)(void* arg, const char* fmt, ...);

static int ziti_dump_to_log_op(void* stringsBuilder, const char* fmt, ...) {
    static char line[MAXBUFFERLEN];

    va_list vargs;
    va_start(vargs, fmt);
    vsnprintf(line, sizeof(line), fmt, vargs);
    va_end(vargs);

    // write/append to the buffer
    strncat(stringsBuilder, line, sizeof(line));
    return 0;
}

static int ziti_dump_to_file_op(void* fp, const char* fmt, ...) {
    static char line[MAXBUFFERLEN];

    va_list vargs;
    va_start(vargs, fmt);
    // write/append to file
    vfprintf(fp, fmt, vargs);
    va_end(vargs);

    return 0;
}

void z4d_ziti_dump_log(ziti_context ztx) {
    char* buffer = malloc(MAXBUFFERLEN * sizeof(char));
    buffer[0] = 0;
    ziti_dump(ztx, (printer)ziti_dump_to_log_op, buffer);
    printf("ziti dump to log %s", buffer);
    free(buffer);
}

void z4d_ziti_dump_file(ziti_context ztx, const char* outputFile) {
    FILE* fp;
    fp = fopen(outputFile, "w+");
    if (fp == NULL)
    {
        printf("ziti dump to file failed. Unable to Read / Write / Create File");
        return;
    }
    uv_timeval64_t dump_time;
    uv_gettimeofday(&dump_time);

    char time_str[32];
    struct tm* start_tm = gmtime(&dump_time.tv_sec);
    strftime(time_str, sizeof(time_str), "%a %b %d %Y, %X %p", start_tm);

    fprintf(fp, "Ziti Dump starting: %s\n", time_str);

    //actually invoke ziti_dump here
    ziti_dump(ztx, (printer)ziti_dump_to_file_op, fp);

    fflush(fp);
    fclose(fp);
}

typedef struct dotnet_callback_s {
    z4d_cb cb;
    void* context;
} dotnet_callback_t;

void onloop(uv_async_t* handle) {
    dotnet_callback_t* dotnetcb = handle->data;
    printf("executing callback on loop\n");
    dotnetcb->cb(dotnetcb->context);
    printf("callback from loop complete\n");
    free(dotnetcb);
    free(handle);
}

void z4d_callback_on_loop(uv_loop_t* loop, void* context, z4d_cb cb) {
    printf("scheduling callback to loop\n");
    uv_async_t* async = calloc(1, sizeof(uv_async_t));
    dotnet_callback_t* dotnetcb = calloc(1, sizeof(dotnet_callback_t));
    dotnetcb->cb = cb;
    dotnetcb->context = context;
    async->data = dotnetcb;
    uv_async_init(loop, async, onloop);
    uv_async_send(async);
}




#define OFFSET(MEMBERNAME) OFFSETNAMED(MEMBERNAME, MEMBERNAME)
#define OFFSETNAMED(MEMBERTYPE, MEMBERNAME) printf("\noffset of ziti_types_v2." #MEMBERTYPE "." #MEMBERNAME ": %zu size: %zu", offsetof(ziti_types_v2, MEMBERNAME), sizeof(MEMBERTYPE))

#define SUBOFFSET(MEMBERTYPE, CHILD) SUBOFFSETNAMED(MEMBERTYPE, MEMBERTYPE, CHILD)
#define SUBOFFSETNAMED(MEMBERTYPE, MEMBERNAME, CHILD) printf("\noffset of ziti_types_v2." #MEMBERNAME "." #CHILD ": %zu.%zu size: %zu", offsetof(ziti_types_v2, MEMBERNAME) + offsetof(MEMBERTYPE, CHILD), offsetof(MEMBERTYPE, CHILD), sizeof(MEMBERTYPE))

#define BYTEALIGNCHECK(FIELDNAME) BYTEALIGNCHECKBYTYPE(FIELDNAME, FIELDNAME)
#define BYTEALIGNCHECKBYTYPE(FIELDNAME, FIELDTYPE) rtn->FIELDNAME.checksum = #FIELDNAME;                      \
                                                   rtn->FIELDNAME.offset = offsetof(ziti_types_v2, FIELDNAME); \
                                                   rtn->FIELDNAME.size = sizeof(FIELDTYPE)
typedef struct c_t {
    uint32_t size;
    ziti_alignment_check_ziti_auth_query_mfa ziti_auth_query_mfa;
} c_s;

ziti_types_v2* z4d_struct_test() {
    ziti_types_v2* rtn = calloc(sizeof(ziti_types_t) + 1, 1);
    rtn->size = sizeof(ziti_types_v2);

    OFFSET(ziti_auth_query_mfa);
    OFFSET(ziti_id_cfg);
    OFFSET(ziti_config);
    OFFSET(api_path);
    OFFSET(ziti_api_versions);
    OFFSET(ziti_version);
    OFFSET(ziti_identity);
    OFFSET(ziti_process);
    OFFSET(ziti_posture_query);
    SUBOFFSETNAMED(ziti_posture_query, ziti_posture_query, id);
    OFFSET(ziti_posture_query_set);
    SUBOFFSET(ziti_posture_query_set, policy_id);
    SUBOFFSET(ziti_posture_query_set, is_passing);
    SUBOFFSET(ziti_posture_query_set, policy_type);
    SUBOFFSET(ziti_posture_query_set, posture_queries);

    OFFSET(ziti_session_type);
    OFFSET(ziti_service);
    OFFSETNAMED(ziti_address, ziti_address_host);
    SUBOFFSETNAMED(ziti_address, ziti_address_host, type);
    SUBOFFSETNAMED(ziti_address, ziti_address_host, addr.hostname);

    OFFSETNAMED(ziti_address, ziti_address_cidr);
    SUBOFFSETNAMED(ziti_address, ziti_address_cidr, type);
    SUBOFFSETNAMED(ziti_address, ziti_address_cidr, addr.cidr.af);
    SUBOFFSETNAMED(ziti_address, ziti_address_cidr, addr.cidr.bits);
    SUBOFFSETNAMED(ziti_address, ziti_address_cidr, addr.cidr.ip);

    OFFSET(ziti_client_cfg_v1);
    OFFSET(ziti_intercept_cfg_v1);
    OFFSET(ziti_server_cfg_v1);
    OFFSET(ziti_listen_options);
    OFFSET(ziti_host_cfg_v1);
    OFFSET(ziti_host_cfg_v2);
    OFFSET(ziti_mfa_enrollment);
    OFFSET(ziti_port_range);

    rtn->ziti_auth_query_mfa_data.type_id = "type";
    rtn->ziti_auth_query_mfa_data.provider = "provider";
    rtn->ziti_auth_query_mfa_data.http_method = "http_method";
    rtn->ziti_auth_query_mfa_data.http_url = "http_url";
    rtn->ziti_auth_query_mfa_data.min_length = 81;
    rtn->ziti_auth_query_mfa_data.max_length = 92;
    rtn->ziti_auth_query_mfa_data.format = "format";
    BYTEALIGNCHECK(ziti_auth_query_mfa);

    rtn->ziti_id_cfg_data.cert = "cert";
    rtn->ziti_id_cfg_data.key = "key";
    rtn->ziti_id_cfg_data.ca = "ca";
    BYTEALIGNCHECK(ziti_id_cfg);

    rtn->ziti_config_data.controller_url = "controller_url";
    rtn->ziti_config_data.id = rtn->ziti_id_cfg_data;
    BYTEALIGNCHECK(ziti_config);

    rtn->api_path_data.path = "path";
    BYTEALIGNCHECK(api_path);

    model_map_set(&rtn->ziti_api_versions_data.edge, "edge_key1", "edge_val1");
    model_map_set(&rtn->ziti_api_versions_data.edge, "edge_key2", "edge_val2");
    BYTEALIGNCHECK(ziti_api_versions);

    rtn->ziti_version_data.version = "version";
    rtn->ziti_version_data.revision = "revision";
    rtn->ziti_version_data.build_date = "build_date";
    rtn->ziti_version_data.api_versions = &rtn->ziti_api_versions_data;
    BYTEALIGNCHECK(ziti_version);

    rtn->ziti_identity_data.id = "id";
    rtn->ziti_identity_data.name = "name";
    model_map_set(&rtn->ziti_identity_data.app_data, "app_data_key1", "app_data_val1");
    model_map_set(&rtn->ziti_identity_data.app_data, "app_data_key2", "app_data_val2");
    BYTEALIGNCHECK(ziti_identity);

    rtn->ziti_process_data.path = "path";
    BYTEALIGNCHECK(ziti_process);

    rtn->ziti_posture_query_data.id = "id";
    rtn->ziti_posture_query_data.is_passing = true;
    rtn->ziti_posture_query_data.query_type = "query_type";
    rtn->ziti_posture_query_data.process = &rtn->ziti_process_data;
    ziti_process_array zpa = calloc(sizeof(ziti_process), 2);
    zpa[0] = &rtn->ziti_process_data;
    rtn->ziti_posture_query_data.processes = zpa;
    rtn->ziti_posture_query_data.timeout = 10;
    int* time_remain = calloc(sizeof(int), 1);
    *time_remain = 20;
    rtn->ziti_posture_query_data.timeoutRemaining = time_remain;
    rtn->ziti_posture_query_data.updated_at = "updated_at";
    BYTEALIGNCHECK(ziti_posture_query);

    rtn->ziti_posture_query_set_data.policy_id = "policy_id";
    rtn->ziti_posture_query_set_data.is_passing = true;
    rtn->ziti_posture_query_set_data.policy_type = "policy_type";
    rtn->ziti_posture_query_set.checksum = "ziti_posture_query_set";
    ziti_posture_query_array zpqa = calloc(sizeof(ziti_posture_query), 2);
    zpqa[0] = &rtn->ziti_posture_query_data;
    rtn->ziti_posture_query_set_data.posture_queries = zpqa;
    ziti_posture_query_set_array zpqsa = calloc(sizeof(ziti_posture_query_set), 2);
    zpqsa[0] = &rtn->ziti_posture_query_set_data;
    BYTEALIGNCHECK(ziti_posture_query_set);
//    rtn->ziti_posture_query_set_checksum = "ziti_posture_query_set";

    rtn->ziti_session_type_data = ziti_session_type_Dial;
    BYTEALIGNCHECK(ziti_session_type);

    rtn->ziti_service_data.id = "id";
    SUBOFFSET(ziti_service, id);
    rtn->ziti_service_data.name = "name";
    SUBOFFSET(ziti_service, name);
    ziti_session_type_array zsta = calloc(sizeof(ziti_session_type), 2);
    zsta[0] = &rtn->ziti_session_type_data;
    rtn->ziti_service_data.permissions = zsta;
    SUBOFFSET(ziti_service, permissions);
    rtn->ziti_service_data.encryption = 1;
    SUBOFFSET(ziti_service, encryption);
    rtn->ziti_service_data.perm_flags = 214;
    SUBOFFSET(ziti_service, perm_flags);
    model_map_set(&rtn->ziti_service_data.config, "config_key1", "config_val1");
    model_map_set(&rtn->ziti_service_data.config, "config_key2", "config_val2");
    SUBOFFSET(ziti_service, config);
    rtn->ziti_service_data.posture_query_set = zpqsa;
    SUBOFFSET(ziti_service, posture_query_set);
    model_map_set(&rtn->ziti_service_data.posture_query_map, "posture_query_map_key1", "posture_query_map_value1");
    model_map_set(&rtn->ziti_service_data.posture_query_map, "posture_query_map_key2", "posture_query_map_value2");
    SUBOFFSET(ziti_service, posture_query_map);
    rtn->ziti_service_data.updated_at = "updated_at";
    SUBOFFSET(ziti_service, updated_at);
    BYTEALIGNCHECK(ziti_service);

    rtn->ziti_address_host_data.type = ziti_address_hostname;
    strncpy(rtn->ziti_address_host_data.addr.hostname, "hostname", 8);
    BYTEALIGNCHECKBYTYPE(ziti_address_host, ziti_address);

    rtn->ziti_address_cidr_data.type = ziti_address_cidr;
    rtn->ziti_address_cidr_data.addr.cidr.af = AF_INET;
    rtn->ziti_address_cidr_data.addr.cidr.bits = 8;
    const char *ip6str = "100.200.50.25";
    if (inet_pton(AF_INET, ip6str, &rtn->ziti_address_cidr_data.addr.cidr.ip) == 1) {
        //printf("successfully parsed\n");
    } else {
        printf("failed to parse ip?\n");
    }
    BYTEALIGNCHECK(ziti_address_cidr);

    rtn->ziti_client_cfg_v1_data.hostname = rtn->ziti_address_host_data;
    rtn->ziti_client_cfg_v1_data.port = 80;
    BYTEALIGNCHECK(ziti_client_cfg_v1);

    ziti_protocol zp = ziti_protocol_tcp;
    model_list_append(&rtn->ziti_intercept_cfg_v1_data.protocols, &zp);
    model_list_append(&rtn->ziti_intercept_cfg_v1_data.addresses, &rtn->ziti_address_host);
    model_list_append(&rtn->ziti_intercept_cfg_v1_data.addresses, &rtn->ziti_address_cidr);

    rtn->ziti_port_range_data.low = 80;
    rtn->ziti_port_range_data.high = 443;

    model_list_append(&rtn->ziti_intercept_cfg_v1_data.port_ranges, &rtn->ziti_port_range);
    model_map_set(&rtn->ziti_intercept_cfg_v1_data.dial_options, "key", "value");
    rtn->ziti_intercept_cfg_v1_data.source_ip = "source_ip";
    BYTEALIGNCHECK(ziti_intercept_cfg_v1);

    rtn->ziti_server_cfg_v1_data.protocol = "protocol";
    rtn->ziti_server_cfg_v1_data.hostname = "hostname";
    rtn->ziti_server_cfg_v1_data.port = 443;
    BYTEALIGNCHECK(ziti_server_cfg_v1);

    rtn->ziti_listen_options_data.bind_with_identity = true;
    duration d = 1000000;
    rtn->ziti_listen_options_data.connect_timeout = d;
    rtn->ziti_listen_options_data.connect_timeout_seconds = 100;
    rtn->ziti_listen_options_data.cost = 9;
    rtn->ziti_listen_options_data.identity = "identity";
    rtn->ziti_listen_options_data.max_connections = 10;
    rtn->ziti_listen_options_data.precendence = "precedence";
    BYTEALIGNCHECK(ziti_listen_options);

    rtn->ziti_host_cfg_v1_data.protocol = "protocol";
    rtn->ziti_host_cfg_v1_data.forward_protocol = true;
    string_array apa = calloc(sizeof(char*), 3);
    apa[0] = "proto1";
    apa[1] = "proto2";
    rtn->ziti_host_cfg_v1_data.allowed_protocols = apa;
    rtn->ziti_host_cfg_v1_data.address = "address";
    rtn->ziti_host_cfg_v1_data.forward_address = true;
    ziti_address_array allowadds = calloc(sizeof(ziti_address), 2);
    allowadds[0] = &rtn->ziti_address_host_data;
    rtn->ziti_host_cfg_v1_data.allowed_addresses = allowadds;
    rtn->ziti_host_cfg_v1_data.port = 1090;
    rtn->ziti_host_cfg_v1_data.forward_port = true;
    ziti_port_range_array zpra = calloc(sizeof(ziti_port_range), 2);
    zpra[0] = &rtn->ziti_port_range_data;
    rtn->ziti_host_cfg_v1_data.allowed_port_ranges = zpra;
    rtn->ziti_host_cfg_v1_data.allowed_source_addresses = allowadds;
    rtn->ziti_host_cfg_v1_data.listen_options = &rtn->ziti_listen_options_data;
    BYTEALIGNCHECK(ziti_host_cfg_v1);

    model_list_append(&rtn->ziti_host_cfg_v2_data.terminators, &rtn->ziti_host_cfg_v2_data);
    BYTEALIGNCHECK(ziti_host_cfg_v2);

    rtn->ziti_mfa_enrollment_data.is_verified = true;
    string_array codes = calloc(sizeof(char*), 2);
    codes[0] = "code1";
    rtn->ziti_mfa_enrollment_data.recovery_codes = codes;
    rtn->ziti_mfa_enrollment_data.provisioning_url = "provisioningUrl";
    BYTEALIGNCHECK(ziti_mfa_enrollment);

    BYTEALIGNCHECK(ziti_port_range);

    rtn->ziti_options_data.config = "config";
    rtn->ziti_options_data.disabled = true;
    SUBOFFSET(ziti_options, disabled);
    char** cfgs = calloc(sizeof(char*), 2);
    cfgs[0] = strdup("config1");
    cfgs[1] = strdup("config2");
    rtn->ziti_options_data.config_types = (const char **) cfgs;
    SUBOFFSET(ziti_options, config_types);
    rtn->ziti_options_data.api_page_size = 232323;
    SUBOFFSET(ziti_options, api_page_size);
    rtn->ziti_options_data.refresh_interval = 3322;
    SUBOFFSET(ziti_options, refresh_interval);
    rtn->ziti_options_data.metrics_type = EWMA_15m;
    SUBOFFSET(ziti_options, metrics_type);
    rtn->ziti_options_data.router_keepalive = 111;
    SUBOFFSET(ziti_options, router_keepalive);
    rtn->ziti_options_data.app_ctx = strdup("ctxhere");
    SUBOFFSET(ziti_options, app_ctx);
    rtn->ziti_options_data.events = 98;
    SUBOFFSET(ziti_options, events);
    BYTEALIGNCHECK(ziti_options);

    /*type is a ziti_event_type*/
    rtn->ziti_context_event_data.type = ZitiContextEvent;
    SUBOFFSETNAMED(ziti_event_t, ziti_context_event, type);
    rtn->ziti_context_event_data.event.ctx.ctrl_status = 245;
    SUBOFFSETNAMED(ziti_event_t, ziti_context_event, event.ctx.ctrl_status);
    rtn->ziti_context_event_data.event.ctx.err = "ziti_context_event_err_0__";
    SUBOFFSETNAMED(ziti_event_t, ziti_context_event, event.ctx.err);
    BYTEALIGNCHECKBYTYPE(ziti_context_event, ziti_event_t);

    rtn->ziti_router_event_data.type = ZitiRouterEvent;
    SUBOFFSETNAMED(ziti_event_t, ziti_router_event, type);
    rtn->ziti_router_event_data.event.router.status = EdgeRouterConnected;
    SUBOFFSETNAMED(ziti_event_t, ziti_router_event, event.router.status);
    rtn->ziti_router_event_data.event.router.name = "ere_name";
    SUBOFFSETNAMED(ziti_event_t, ziti_router_event, event.router.name);
    rtn->ziti_router_event_data.event.router.address = "ere_address";
    SUBOFFSETNAMED(ziti_event_t, ziti_router_event, event.router.address);
    rtn->ziti_router_event_data.event.router.version = "ere_version";
    SUBOFFSETNAMED(ziti_event_t, ziti_router_event, event.router.version);
    BYTEALIGNCHECKBYTYPE(ziti_router_event, ziti_event_t);

    rtn->ziti_service_event_data.type = ZitiServiceEvent;
    SUBOFFSETNAMED(ziti_event_t, ziti_service_event, type);
    ziti_service_array list_of_services = calloc(sizeof(ziti_service*), 3);
    list_of_services[0] = &rtn->ziti_service_data;
    ziti_service* elem2 = calloc(sizeof(ziti_service), 1);
    list_of_services[0]->name = strdup("elem1");
    list_of_services[0]->id = strdup("elem1id");
    list_of_services[0]->perm_flags = 111;
    elem2->name = strdup("elem2");
    elem2->id = strdup("elem2id");
    elem2->perm_flags = 222;
    list_of_services[1] = elem2; //&rtn->ziti_service_data;
    rtn->ziti_service_event_data.event.service.added = list_of_services;
    SUBOFFSETNAMED(ziti_event_t, ziti_service_event, event.service.added);
    rtn->ziti_service_event_data.event.service.changed = list_of_services;
    SUBOFFSETNAMED(ziti_event_t, ziti_service_event, event.service.changed);
    rtn->ziti_service_event_data.event.service.removed = list_of_services;
    SUBOFFSETNAMED(ziti_event_t, ziti_service_event, event.service.removed);
    BYTEALIGNCHECKBYTYPE(ziti_service_event, ziti_event_t);

    rtn->ziti_mfa_auth_event_data.type = ZitiMfaAuthEvent;
    SUBOFFSETNAMED(ziti_event_t, ziti_mfa_auth_event, type);
    rtn->ziti_mfa_auth_event_data.event.mfa_auth_event.auth_query_mfa = &rtn->ziti_auth_query_mfa_data;
    SUBOFFSETNAMED(ziti_event_t, ziti_mfa_auth_event, event.mfa_auth_event.auth_query_mfa);
    BYTEALIGNCHECKBYTYPE(ziti_mfa_auth_event, ziti_event_t);

    rtn->ziti_api_event_data.type = ZitiAPIEvent;
    SUBOFFSETNAMED(ziti_event_t, ziti_api_event, type);
    rtn->ziti_api_event_data.event.api.new_ctrl_address = "new_ctrl_address";
    SUBOFFSETNAMED(ziti_event_t, ziti_api_event, event.api.new_ctrl_address);
    rtn->ziti_api_event_data.event.api.new_ca_bundle = "new_ca_bundle";
    SUBOFFSETNAMED(ziti_event_t, ziti_api_event, event.api.new_ca_bundle);
    BYTEALIGNCHECKBYTYPE(ziti_api_event, ziti_event_t);

    printf("\noffset output complete\n\n");

    return rtn;
}

ziti_posture_query* z4d_ziti_posture_query() {
    ziti_posture_query* q = calloc(sizeof(ziti_posture_query), 1);
    q->id = "id";
    q->is_passing = true;
    q->query_type = "query_type";
    printf("rtn->ziti_posture_query.query_type memory location: %p\n", &q->query_type);

    return q;
}

void useUnusedFuncs() {
    //TODO: temporary hack to get the linker to emit 'unused' symbols
    ziti_enroll(NULL, NULL, NULL, NULL);
    ziti_conn_bridge(NULL, NULL, NULL);
    ziti_conn_bridge_fds(NULL, NULL, NULL, NULL, NULL);
    Ziti_lib_init();
}

