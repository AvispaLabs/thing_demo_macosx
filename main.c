#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kii_sources/kii_cloud.h"

void get_cpu_temp(char* out_temp, size_t buff_len)
{
    char *ptr = NULL;
    FILE *fp = NULL;
    fp = popen("osx-cpu-temp", "r");
    if (fp == NULL) {
        fprintf(stderr, "failed to execute command.\n");
        exit(-1);
    }

    fgets(out_temp, buff_len, fp);
    ptr = strstr(out_temp, "°C");
    if (ptr != NULL) {
        *ptr = '\0';
    }

    pclose(fp);
}

const char* g_app_id = "842e00d8";
const char* g_app_key = "851995d3247d81d5fd771b8752adddb1";
const char* g_base_url = "https://api.kii.com/api";

char g_vendor_thing_id[128];
const char* g_thing_password = "S0xAUJqr";
const char* g_thing_bucket = "tempertures";

kii_app_t g_app = NULL;
kii_thing_t g_thing = NULL;
kii_char_t* g_token = NULL;

void register_thing() {
    pid_t pid = getpid();
    g_vendor_thing_id[0] = '\0';
    snprintf(g_vendor_thing_id, 128, "thingid_%d", pid);
    printf("thing id: %s\n", g_vendor_thing_id);
    kii_error_code_t ret = kii_register_thing(g_app,
            g_vendor_thing_id,
            g_thing_password,
            NULL,
            NULL,
            &g_thing,
            &g_token);

    if (ret != KIIE_OK) {
        fprintf(stderr, "failed to execute command.\n");
        exit(-1);
    }

    /* store thing */
    kii_char_t* sThing = kii_thing_serialize(g_thing);
    char command[256];
    snprintf(command, 256, "echo %s > thing.dat", sThing);
    system(command);

    /* store token */
    char command2[256];
    snprintf(command2, 256, "echo %s > token.dat", g_token);
    system(command2);

    /* store QR */
    char command3[256];
    snprintf(command3, 256, "qrencode -o qr.png `echo %s,%s`", sThing, g_token);
    system(command3);
}

void save_temperture(double temperture) {
    json_t* contents = json_object();
    json_object_set_new(contents, "cpu-temp", json_real(temperture));

    kii_bucket_t tempBucket = kii_init_thing_bucket(g_thing, g_thing_bucket);
    kii_error_code_t ret = kii_create_new_object(g_app,
            g_token,
            tempBucket,
            contents,
            NULL,
            NULL);

    if (ret != KIIE_OK) {
        fprintf(stderr, "failed to save object.\n");
        kii_error_t* err = kii_get_last_error(g_app);
        fprintf(stderr, "error: http %d %s.\n", err->status_code, err->error_code);
        exit(-1);
    }
    json_decref(contents);
    kii_dispose_bucket(tempBucket);
}

int load_thing() {
    char thingStr[256];
    char* temp = NULL;
    FILE *fp1 = NULL;
    fp1 = fopen("thing.dat", "r");
    if (fp1 == NULL) {
        return 0;
    }

    fgets(thingStr, 256, fp1);
    temp = strchr(thingStr, '\n');
    if (temp != NULL) {
        *temp = '\0';
    }
    fclose(fp1);

    printf("loaded : %s\n", thingStr);
    g_thing = kii_thing_deserialize(thingStr);
    return 1;
}

int load_token() {
    char tokenStr[256];
    char* temp = NULL;
    FILE *fp1 = NULL;
    fp1 = fopen("token.dat", "r");
    if (fp1 == NULL) {
        return 0;
    }

    fgets(tokenStr, 256, fp1);
    temp = strchr(tokenStr, '\n');
    if (temp != NULL) {
        *temp = '\0';
    }
    fclose(fp1);

    g_token = strdup(tokenStr);
    return 1;
}

int main()
{
    kii_global_init();

    g_app = kii_init_app(g_app_id, g_app_key, g_base_url);

    /* Load registered thing or register new thing */
    int loadSuc = load_thing();
    if (loadSuc == 0) {
        register_thing();
    }
    load_token();

    /* Record CPU temperture */
    char str[128];
    get_cpu_temp(str, 128);
    printf("temperature: %s°C\n", str);
    double dtemp = atof(str);

    save_temperture(dtemp);

    /* clean up */
    kii_dispose_app(g_app);
    kii_dispose_thing(g_thing);
    kii_dispose_kii_char(g_token);
    kii_global_cleanup();
    return 0;
}

