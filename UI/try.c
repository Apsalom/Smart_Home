#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include<sqlite3.h>
#include"interface.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int main() {

	printf("resa\n");
    sqlite3 *db;
    char *err_msg = 0;
    int *a = 0;
    int rc = sqlite3_open("Smart.db", &db);

    if (rc != SQLITE_OK) {

        printf("adsdf\n");
	fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }
    char *sql =NULL;
    int b= get_temperature();
    int c= get_moisture();
    int d= get_distance();
    char e[20]; 
    int ej= is_available();
    if(ej==1)
    {
      strcpy(e,"Available");
    }
    else{
      strcpy(e,"Not_Available");
     }
    c=(((float)c/4096)*100);
    printf("%s\n",e);
    asprintf(&sql, "update  Smart set Temp=%d,mois=%d,water=%d,Avail='%s';",b,c,d,e);         
    rc = sqlite3_exec(db, sql,0 ,0,&err_msg);

    if (rc != SQLITE_OK ) {

        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);

        return 1;
    }

  //  printf("table created");
    sqlite3_close(db);

    return 0;
}
