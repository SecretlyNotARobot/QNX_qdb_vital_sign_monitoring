#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <qdb/qdb.h>

/**
 * This sample program connects to the database and does one INSERT and one
 * SELECT.
 *
 * The database name is assumed to be /dev/qdb/customerdb
 * with schema:
 *    CREATE TABLE customers(
 *       customerid INTEGER PRIMARY KEY AUTOINCREMENT,
 *       firstname  TEXT,
 *       lastname   TEXT
 *     );
 */

//CREATE TABLE customers(
//   customerid INTEGER PRIMARY KEY AUTOINCREMENT,
//   firstname  TEXT,
//   lastname   TEXT
//);

int main(int argc, char **argv) {
   int rc;
   qdb_hdl_t *hdl;
   qdb_result_t *res;
   char *errmsg;

   // Connect to the database
   hdl = qdb_connect("/tmp/customerdb.db", 0);
   if (hdl == NULL){
      fprintf(stderr, "Error connecting to database: %s\n", strerror(errno));
      return EXIT_FAILURE;
   }

   // INSERT a row into the database.
   rc = qdb_statement(hdl,
      "INSERT INTO customers(firstname, lastname) VALUES('AJ', 'Ricketts');");
   if (rc == -1) {
      errmsg = qdb_geterrmsg(hdl);
      fprintf(stderr, "Error executing INSERT statement: %s\n", errmsg);
      return EXIT_FAILURE;
   }

   // SELECT one row from the database
   // This statement combines the first and last names together into their
   // full name.
   rc = qdb_statement(hdl,
         "SELECT firstname || ' ' || lastname AS fullname FROM customers LIMIT 1;");
   if (rc == -1) {
      errmsg = qdb_geterrmsg(hdl);
      fprintf(stderr, "Error executing SELECT statement: %s\n", errmsg);
      return EXIT_FAILURE;
   }
   res = qdb_getresult(hdl); // Get the result
   if (res == NULL) {
      fprintf(stderr, "Error getting result: %s\n", strerror(errno));
      return EXIT_FAILURE;
   }
   if (qdb_rows(res) == 1) {
      printf("Got a customer's full name: %s\n", (char *)qdb_cell(res, 0, 0));
   }
   else {
      printf("No customers in the database!\n");
   }
   // Free the result
   qdb_freeresult(res);

   // Disconnect from the sever
   qdb_disconnect(hdl);

   return EXIT_SUCCESS;
}
