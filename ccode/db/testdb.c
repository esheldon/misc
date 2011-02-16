#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <db.h>

// for memset
#include <string.h>

#define DBSIZE 10000000

int
compare_float(DB *dbp, const DBT *a, const DBT *b)
{
    float ai, bi;

    /* 
     * Returns: 
     * < 0 if a < b 
     * = 0 if a = b 
     * > 0 if a > b 
     */ 
    memcpy(&ai, a->data, sizeof(float)); 
    memcpy(&bi, b->data, sizeof(float)); 

	return (ai > bi) - (ai < bi);

} 



int
compare_int32(DB *dbp, const DBT *a, const DBT *b)
{
    int32_t ai, bi;

    /* 
     * Returns: 
     * < 0 if a < b 
     * = 0 if a = b 
     * > 0 if a > b 
     */ 
    memcpy(&ai, a->data, sizeof(int32_t)); 
    memcpy(&bi, b->data, sizeof(int32_t)); 
    return (ai - bi); 
} 

int print_match_S20(DB *dbp, char* cvalue) {
	// The database cursor
	DBC *cursorp;

	DBT key_dbt, data_dbt;

	int ret;

	int32_t index;
	char key[20];

	int32_t count=0;

	// Use the memory in key for the searching, this will make
	// for cleaner code in the loop, also possibly help with
	// alignment issues
	
	strncpy(key, cvalue, 20);

	memset(&key_dbt, 0, sizeof(DBT));
	key_dbt.data = &key;
	key_dbt.size = 20;
	key_dbt.flags = DB_DBT_USERMEM;
	key_dbt.ulen = 20;

	memset(&data_dbt, 0, sizeof(data_dbt));
	data_dbt.data = &index;
	data_dbt.size = sizeof(int32_t);
	data_dbt.flags = DB_DBT_USERMEM;
	data_dbt.ulen = sizeof(int32_t);



	/* Now, create a cursor. */
	if((ret = dbp->cursor(dbp, NULL, &cursorp, 0)) != 0){
		dbp->err(dbp, ret, "DB->cursor failed.");
		return (ret);
	}

	printf("Searching for exact match to: %s\n", (char*)(key_dbt.data));

	for (ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_SET_RANGE);
			ret == 0;
			ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_NEXT)) {

	    /* Check if we are still in the range. */
	    if ( strncmp(key, cvalue, 20) != 0 ) {
	    	break;
		}
	
		// the beginning of the data is the key
		//printf("Key: %s  index: %d\n", key, index);
		count ++;
	}

	printf("\nFound %d matches\n", count);

	if(cursorp != NULL) {
		cursorp->c_close(cursorp);
	}
	return(0);

}



int print_between_int32(DB *dbp, int32_t low, int32_t high) {
	// The database cursor
	DBC *cursorp;

	DBT key_dbt, data_dbt;

	int ret;

	int32_t* fptr;
	int32_t index;

	int32_t key=0;

	int32_t count=0;

	// Use the memory in key for the searching, this will make
	// for cleaner code in the loop, also possibly help with
	// alignment issues
	
	key = low;

	memset(&key_dbt, 0, sizeof(DBT));
	key_dbt.data = &key;
	key_dbt.size = sizeof(int32_t);
	key_dbt.flags = DB_DBT_USERMEM;
	key_dbt.ulen = sizeof(int32_t);

	memset(&data_dbt, 0, sizeof(data_dbt));
	data_dbt.data = &index;
	data_dbt.size = sizeof(int32_t);
	data_dbt.flags = DB_DBT_USERMEM;
	data_dbt.ulen = sizeof(int32_t);



	/* Now, create a cursor. */
	if((ret = dbp->cursor(dbp, NULL, &cursorp, 0)) != 0){
		dbp->err(dbp, ret, "DB->cursor failed.");
		return (ret);
	}

	if (low == high) {
		printf("Searching for exact match to: %d\n", *(int32_t*)(key_dbt.data));
	} else {
		printf("Searching for %d <= data < %d\n", *(int32_t*)(key_dbt.data), high);
	}

	/* Now loop over items that match exactly. */
	// note hashes ignore the SETRANGE and just use SET
	for (ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_SET_RANGE);
			ret == 0;
			ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_NEXT)) {


	    /* Check if we are still in the range. */
	    if (key > high) {
	    	break;
		}
	
		// the beginning of the data is the key
		//printf("Key: %d  index: %d\n", key, index);
		count ++;
	}

	printf("\nFound %d matches\n", count);

	if(cursorp != NULL) {
		cursorp->c_close(cursorp);
	}
	return(0);

}



int print_between_float(DB *dbp, float low, float high) {
	// The database cursor
	DBC *cursorp;

	DBT key_dbt, data_dbt;

	int ret;

	float* fptr;
	int32_t index;

	float key=0;

	int32_t count=0;

	// Use the memory in key for the searching, this will make
	// for cleaner code in the loop, also possibly help with
	// alignment issues
	key = low;
	memset(&key_dbt, 0, sizeof(DBT));
	key_dbt.data = &key;
	key_dbt.size = sizeof(float);
	key_dbt.flags = DB_DBT_USERMEM;
	key_dbt.ulen = sizeof(float);

	memset(&data_dbt, 0, sizeof(data_dbt));
	data_dbt.data = &index;
	data_dbt.size = sizeof(int32_t);
	data_dbt.flags = DB_DBT_USERMEM;
	data_dbt.ulen = sizeof(int32_t);



	/* Now, create a cursor. */
	if((ret = dbp->cursor(dbp, NULL, &cursorp, 0)) != 0){
		dbp->err(dbp, ret, "DB->cursor failed.");
		return (ret);
	}

	printf("Searching for %f <= data < %f\n", *(float*)(key_dbt.data), high);

	/* Now loop over items starting with the low key. */
	for (ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_SET_RANGE);
			ret == 0;
			ret = cursorp->c_get(cursorp, &key_dbt, &data_dbt, DB_NEXT)) {


	    /* Check if we are still in the range. */
	    if (key > high) {
	    	break;
		}
	
		// the beginning of the data is the key
		printf("Key: %e  index: %d\n", key, index);
		count ++;
	}

	printf("\nFound %d matches\n", count);

	if(cursorp != NULL) {
		cursorp->c_close(cursorp);
	}
	return(0);

}


int print_S20_records(DB* dbp) {

	DBC *cursorp;

	// DBT stands for data base thang
	DBT row_key, row_data;

	int32_t index=0;
	char key[20];
	int32_t data=0;

	int ret=0;

	//int step=1000;
	int step=1;

	if((ret = dbp->cursor(dbp, NULL, &cursorp, 0)) != 0){
		dbp->err(dbp, ret, "DB->cursor failed.");
		return (ret);
	}

	printf("Printing some S20 records\n");

	/* Zero out the DBTs before using them. */
	memset(&row_key, 0, sizeof(DBT));
	memset(&row_data, 0, sizeof(DBT));


	// we use our own memory just in case of alignment problems with some
	// architecture, probably not an issue but it's also not much work

	row_data.data = &data;
	row_data.flags = DB_DBT_USERMEM;
	row_data.ulen = sizeof(int32_t);

	row_key.data = &key;
	row_key.flags = DB_DBT_USERMEM;
	row_key.ulen = 20;

	while ((ret = cursorp->c_get(cursorp, &row_key, &row_data, DB_NEXT)) == 0) {
		if ( ((index+1) % step) == 0) {
			printf("Retrieved key: %s data: %d\n", key, data);
		}
		index += 1;
	}


	return(ret);

	ret = cursorp->c_get(cursorp, &row_key, &row_data, DB_FIRST);
	while (ret == 0) {
		if ( ((index+1) % step) == 0) {
			printf("Retrieved key: %f data: %d\n", key, data);
		}

		ret = cursorp->c_get(cursorp, &row_key, &row_data, DB_NEXT);

		index += 1;
	}

}



int print_float_records(DB* dbp) {

	DBC *cursorp;

	// DBT stands for data base thang
	DBT row_key, row_data;

	int32_t index=0;
	float key=0;
	int32_t data=0;

	int ret=0;

	//int step=1000;
	int step=1;

	if((ret = dbp->cursor(dbp, NULL, &cursorp, 0)) != 0){
		dbp->err(dbp, ret, "DB->cursor failed.");
		return (ret);
	}

	printf("Printing some float records\n");

	/* Zero out the DBTs before using them. */
	memset(&row_key, 0, sizeof(DBT));
	memset(&row_data, 0, sizeof(DBT));


	// we use our own memory just in case of alignment problems with some
	// architecture, probably not an issue but it's also not much work

	row_data.data = &data;
	row_data.flags = DB_DBT_USERMEM;
	row_data.ulen = sizeof(int32_t);

	row_key.data = &key;
	row_key.flags = DB_DBT_USERMEM;
	row_key.ulen = sizeof(int);

	while ((ret = cursorp->c_get(cursorp, &row_key, &row_data, DB_NEXT)) == 0) {
		if ( ((index+1) % step) == 0) {
			printf("Retrieved key: %f data: %d\n", key, data);
		}
		index += 1;
	}


	return(ret);

	ret = cursorp->c_get(cursorp, &row_key, &row_data, DB_FIRST);
	while (ret == 0) {
		if ( ((index+1) % step) == 0) {
			printf("Retrieved key: %f data: %d\n", key, data);
		}

		ret = cursorp->c_get(cursorp, &row_key, &row_data, DB_NEXT);

		index += 1;
	}

    // Cursors must be closed
    if (cursorp != NULL)  {
        cursorp->close(cursorp); 
    }

}

int add_S20_records(DB* dbp) {
	// add some int key, float data to the database
	// DBT stands for data base thang
	DBT key_dbt, data_dbt;

	char key[20];
	int32_t data=0;

	int32_t index=0;
	int ret;
	int step = 10000;

	FILE* fptr;

	fptr = fopen("S20-10000000.bin","r");

	/* Zero out the DBTs before using them. */
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));

	key_dbt.size = 20;
	data_dbt.size = sizeof(int32_t);

	for (index=0; index<DBSIZE; index++) {

		// just set data to i + 0.2 for test
		data = index;

		fread(&key, 20, 1, fptr);

		key_dbt.data = &key;
		data_dbt.data = &data;

		ret = dbp->put(dbp, NULL, &key_dbt, &data_dbt, 0);
		if (ret != 0) {
			dbp->err(dbp, ret, "Put failed because:");
		}

		if ( ((index+1) % step) == 0) {
			printf("Added row number: %d/%d  key: %s\n", index+1, DBSIZE, key);
		}
	}

	fclose(fptr);

}




int add_int32_records(DB* dbp) {
	// add some int key, float data to the database
	// DBT stands for data base thang
	DBT key_dbt, data_dbt;

	int32_t key=0;
	int32_t data=0;

	int32_t index=0;
	int ret;
	int step = 10000;

	FILE* fptr;

	fptr = fopen("random-int32-10000000.bin","r");

	/* Zero out the DBTs before using them. */
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));

	key_dbt.size = sizeof(int32_t);
	data_dbt.size = sizeof(int32_t);

	for (index=0; index<DBSIZE; index++) {

		// just set data to i + 0.2 for test
		data = index;

		fread(&key, sizeof(int32_t), 1, fptr);

		key_dbt.data = &key;
		data_dbt.data = &data;

		ret = dbp->put(dbp, NULL, &key_dbt, &data_dbt, 0);
		if (ret != 0) {
			dbp->err(dbp, ret, "Put failed because:");
		}

		if ( ((index+1) % step) == 0) {
			printf("Added row number: %d/%d  key: %d\n", index+1, DBSIZE, key);
		}
	}

	fclose(fptr);

}



int add_float_records(DB* dbp) {
	// add some int key, float data to the database
	// DBT stands for data base thang
	DBT key_dbt, data_dbt;

	float key=0;
	int32_t data=0;

	int32_t index=0;
	int ret;
	int step = 10000;

	FILE* fptr;

	fptr = fopen("random-float-10000000.bin","r");

	/* Zero out the DBTs before using them. */
	memset(&key_dbt, 0, sizeof(DBT));
	memset(&data_dbt, 0, sizeof(DBT));

	key_dbt.size = sizeof(float);
	data_dbt.size = sizeof(int32_t);

	for (index=0; index<DBSIZE; index++) {

		data = index;

		fread(&key, sizeof(float), 1, fptr);

		key_dbt.data = &key;
		data_dbt.data = &data;

		// would do this if no dups were allowed
		/*ret = dbp->put(dbp, NULL, &key_dbt, &data_dbt, DB_NOOVERWRITE);
		if (ret == DB_KEYEXIST) {
			dbp->err(dbp, ret, 
					"Put failed because key %d already exists", index);
		}
		*/
		ret = dbp->put(dbp, NULL, &key_dbt, &data_dbt, 0);
		if (ret != 0) {
			dbp->err(dbp, ret, "Put failed because:");
		}

		if ( ((index+1) % step) == 0) {
			printf("Added row number: %d/%d  key: %f\n", index+1, DBSIZE, key);
		}
	}

	fclose(fptr);

}

// This version doesn't allow duplicate keys yet
void print_usage() {
	printf("testdb database_type data_type action [value/low high]\n");

	printf("  If database_type=1 btree, 2 hash\n");
	printf("  If data_type=1, int32, 2 float, 3 S20\n");
	printf("  If action=1, load appropriate data\n");
	printf("  If action=2, either an integer value for the hash\n");
	printf("     or low and high for the btree\n");
	printf("  If action=3 print all data\n");
}

void print_usage_exit() {
	print_usage();
	exit(45);
}

int main(int argc, char** argv) {

	DB* dbp=NULL; /* pointer to DB structure */
	u_int32_t flags; /* database open flags */
	int ret=0;

	char dbname[100];
	// 1 for bree 2 for hash
	int database_type=0;

	// 1 for int32 2 for float
	int data_type=0;

	// 1 means add the rows, 2 means perform a search
	int action=0;

	int32_t low_int32=0, high_int32=0;
	float low_float=0, high_float=0;
	char cvalue[20];
	int retvalue=0;

	if (argc < 4) {
		print_usage_exit();
	}

	database_type = atoi(argv[1]);
	data_type = atoi(argv[2]);
	action = atoi(argv[3]);


	// get the right database name
	if (database_type == 1) {
		// btree
		if (data_type == 1) {
			strcpy(dbname, "btree-int32.db");
		} else if (data_type == 2) {
			strcpy(dbname, "btree-float.db");
		} else if (data_type == 3) {
			strcpy(dbname, "btree-S20.db");
		} else {
			print_usage_exit();
		}
	} else if (database_type == 2) {
		// hash
		if (data_type == 1) {
			strcpy(dbname, "hash-int32.db");
		} else if (data_type == 2) {
			printf("Cannot do float keys for hash\n");
			print_usage_exit();
		} else if (data_type == 3) {
			strcpy(dbname, "hash-S20.db");
		} else {
			print_usage_exit();
		}
	} else {
		print_usage_exit();
	}

	// Should we perform a search
	if (action == 2) {
		if (data_type == 1) {
			// int32
			if (argc < 5) {
				print_usage_exit();
			}
			low_int32 = atoi(argv[4]);
			if (argc >= 6) {
				high_int32 = atoi(argv[5]);
			} else {
				high_int32 = low_int32;
			}
			printf("lo: %d  high: %d\n", low_int32, high_int32);
		} else if (data_type == 2) {
			// float
			if (argc < 6) {
				print_usage_exit();
			}
			low_float = atof(argv[4]);
			high_float = atof(argv[5]);
			printf("low: %f  high: %f\n", low_float, high_float);
		} else if (data_type == 3) {
			// char data
			if (argc < 5) {
				print_usage_exit();
			}
			strcpy(cvalue, argv[4]);
		}
	}

	/* Initialize the database structure. This database is not opened in an
	 * environment, so the environment pointer is NULL. */

	ret = db_create(&dbp, NULL, 0);
	if (ret != 0) {
	  printf("Error creating database structure\n");
	  exit(45);
	}



	/*
	 *   Configure the database for sorted duplicates
	 *   Must call this before the open command.
	 */
	ret = dbp->set_flags(dbp, DB_DUPSORT);
	if (ret != 0) {
		dbp->err(dbp, ret, "Attempt to set DUPSORT flag failed.");
		dbp->close(dbp, 0);
		return(ret);
	}




	/* Database open flags */
	flags = DB_CREATE;    /* If the database does not exist, 
						   * create it.*/

	if (database_type == 1) {
		// btree
		if (data_type == 1) {
			dbp->set_bt_compare(dbp, compare_int32);
		} else if (data_type == 2) {
			dbp->set_bt_compare(dbp, compare_float);
		}
		// for strings we use default compare
	} else if (database_type == 2) {
		// hash
		if (data_type == 1) {
			// set_h_compare must be new, since doesn't work
			// on bach00.  I don't think we need it anyway since
			// hashes can only look up exact matches
			//dbp->set_h_compare(dbp, compare_int32);
		} else {
			//dbp->set_h_compare(dbp, compare_float);
		}
	}

	// always have int32 as index in "data" section
	printf("Setting dup comparison function to int32\n");
	dbp->set_dup_compare(dbp, compare_int32);

	if (database_type == 1) {
		// open the database as btree
		printf("Opening '%s'\n", dbname);
		ret = dbp->open(dbp,        // DB structure pointer
				NULL,       // Transaction pointer
				dbname,     // On-disk file that holds the database. 
				NULL,       // Optional logical database name 
				DB_BTREE,   // Database access method 
				flags,      // Open flags 
				0);         // File mode (using defaults) 

	} else if (database_type == 2) {
		// open as hash db
		printf("Opening '%s'\n", dbname);
		ret = dbp->open(dbp,        // DB structure pointer 
				NULL,       // Transaction pointer 
				dbname,     // On-disk file that holds the database. 
				NULL,       // Optional logical database name 
				DB_HASH,    // Database access method 
				flags,      // Open flags
				0);         // File mode (using defaults) 
	} 


	if (ret != 0) {
		printf("Error opening database\n");
		exit(45);
	}




	if (action == 1) {
		// Add some data
		if (data_type == 1) {
			add_int32_records(dbp);
		} else if (data_type == 2) {
			add_float_records(dbp);
		} else if (data_type == 3) {
			add_S20_records(dbp);
		}
	} else if (action == 2) {
		// print matching data
		if (data_type == 1) {
			print_between_int32(dbp, low_int32, high_int32);
		} else if (data_type == 2) {
			print_between_float(dbp, low_float, high_float);
		} else {
			print_match_S20(dbp, cvalue);
		}
	} else {
		if (data_type == 1) {
			printf("Implement print_int32_records\n");
		} else if (data_type == 2) {
			print_float_records(dbp);
		} else if (data_type == 3) {
			print_S20_records(dbp);
		}
	}

	if (dbp != NULL) {
		dbp->close(dbp, 0); 
	}

	return(retvalue);
}
