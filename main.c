#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kii_sources/kii_cloud.h"

void get_cpu_temp(char* out_temp)
{
    char *ptr = NULL;
    FILE *fp = NULL;
    fp = popen("osx-cpu-temp", "r");
    if (fp == NULL) {
        fprintf(stderr, "failed to execute command.\n");
        exit(-1);
    }
    while(1) {
        fgets(out_temp, 512, fp);
        if (feof(fp)) {
            break;
        }
        ptr = strstr(out_temp, "°C");
        if (ptr != NULL) {
            *ptr = '\0';
        }
    }
    pclose(fp);
}

const char* appId = "616962c4";
const char* appKey = "fd02f617b070ec1ae9819935e3943bd6";
const char* baseUrl = "https://api-jp.kii.com/api";

const char* vendorThingId = "thing-demo-2";
const char* thingPassword = "S0xAUJqr";

int retistered = 0;
kii_app_t app = NULL;
kii_thing_t thing = NULL;
kii_char_t* token = NULL;

void register_thing() {

    kii_error_code_t ret = kii_register_thing(app,
            vendorThingId,
            thingPassword,
            NULL,
            NULL,
            &thing,
            &token);

    if (ret != KIIE_OK) {
        fprintf(stderr, "failed to execute command.\n");
        exit(-1);
    }

    /* store thing */
    kii_char_t* sThing = kii_thing_serialize(thing);
    char command[256];
    sprintf(command, "echo %s > thing.dat", sThing);
    system(command);

    /* store token */
    char command2[256];
    sprintf(command2, "echo %s > token.dat", token);
    system(command2);
}

int load_thing() {
    char thingStr[256];
    char* temp = NULL;
    FILE *fp1 = NULL;
    fp1 = fopen("thing.dat", "r");
    if (fp1 == NULL) {
        return 0;
    }
    while(1) {
        fgets(thingStr, 256, fp1);
        if (feof(fp1)) {
            break;
        }
        temp = strchr(thingStr, '\n');
        if (temp != NULL) {
            *temp = '\0';
        }
    }
    fclose(fp1);
    printf("thingStr: %s", thingStr);
    thing = kii_thing_deserialize(thingStr);
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
    while(1) {
        fgets(tokenStr, 256, fp1);
        if (feof(fp1)) {
            break;
        }
        temp = strchr(tokenStr, '\n');
        if (temp != NULL) {
            *temp = '\0';
        }
    }
    fclose(fp1);
    printf("tokenStr: %s", tokenStr);
    token = strdup(tokenStr);
    return 1;
}

int main()
{
    kii_global_init();

    app = kii_init_app(appId, appKey, baseUrl);

    /* Load registered thing or register new thing */
    int loadSuc = load_thing();
    if (loadSuc == 0) {
        register_thing();
    }
    load_token();

    /* Record CPU temperture */
    char str[512];
    get_cpu_temp(str);
    printf("%s\n", str);
    double dtemp = atof(str);

    json_t* contents = json_object();
    json_object_set_new(contents, "cpu-temp", json_real(dtemp));

    kii_bucket_t tempBucket = kii_init_thing_bucket(thing, "tempertures");
    kii_error_code_t ret = kii_create_new_object(app,
            token,
            tempBucket,
            contents,
            NULL,
            NULL);

    if (ret != KIIE_OK) {
        fprintf(stderr, "failed to save object.\n");
        kii_error_t* err = kii_get_last_error(app);
        fprintf(stderr, "error: %d %s.\n", err->status_code, err->error_code);
        exit(-1);
    }
    json_decref(contents);
    kii_dispose_bucket(tempBucket);

    kii_global_cleanup();
    return 0;
}

