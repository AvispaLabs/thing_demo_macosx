#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

const char* g_app_id = "616962c4";
const char* g_app_key = "fd02f617b070ec1ae9819935e3943bd6";
const char* g_base_url = "https://api-jp.kii.com/api";

const char* g_vendor_thing_id = "thing-demo-3";
const char* g_thing_password = "S0xAUJqr";

kii_app_t g_app = NULL;
kii_thing_t g_thing = NULL;
kii_char_t* g_token = NULL;

void register_thing() {

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
    sprintf(command, "echo %s > thing.dat", sThing);
    system(command);

    /* store token */
    char command2[256];
    sprintf(command2, "echo %s > token.dat", g_token);
    system(command2);
}

void save_temperture(double temperture) {
    json_t* contents = json_object();
    json_object_set_new(contents, "cpu-temp", json_real(temperture));

    kii_bucket_t tempBucket = kii_init_thing_bucket(g_thing, "tempertures");
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

    printf("thingStr: %s", thingStr);
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

    printf("tokenStr: %s", tokenStr);
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
    printf("%s\n", str);
    double dtemp = atof(str);

    save_temperture(dtemp);

    /* clean up */
    kii_dispose_app(g_app);
    kii_dispose_thing(g_thing);
    kii_dispose_kii_char(g_token);
    kii_global_cleanup();
    return 0;
}

