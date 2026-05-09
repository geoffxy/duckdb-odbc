#include "odbc_test_common.h"

using namespace odbc_test;

#define ROW_ARRAY_SIZE 5

TEST_CASE("Test column-wise fetch of TIMESTAMP array binding", "[odbc]") {
    SQLHANDLE env;
    SQLHANDLE dbc;
    HSTMT hstmt = SQL_NULL_HSTMT;

    CONNECT_TO_DATABASE(env, dbc);
    EXECUTE_AND_CHECK("SQLAllocHandle (HSTMT)", hstmt, SQLAllocHandle, SQL_HANDLE_STMT, dbc, &hstmt);

    EXECUTE_AND_CHECK("SQLExecDirect", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("DROP TABLE IF EXISTS ts_col_test"), SQL_NTS);
    EXECUTE_AND_CHECK("SQLExecDirect", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("CREATE TABLE ts_col_test (col1 TIMESTAMP)"), SQL_NTS);

    for (int i = 0; i < ROW_ARRAY_SIZE; ++i) {
        char buf[128];
        snprintf(buf, sizeof(buf), "INSERT INTO ts_col_test VALUES ('2000-01-01 00:00:%02d')", i);
        EXECUTE_AND_CHECK("SQLExecDirect (INSERT)", hstmt, SQLExecDirect, hstmt, ConvertToSQLCHAR(buf), SQL_NTS);
    }

    SQL_TIMESTAMP_STRUCT ts_array[ROW_ARRAY_SIZE];
    SQLLEN ts_ind[ROW_ARRAY_SIZE];
    SQLUSMALLINT row_array_status[ROW_ARRAY_SIZE];
    SQLULEN rows_fetched = 0;

    // Column-wise: SQL_BIND_BY_COLUMN (default)
    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROW_ARRAY_SIZE)", hstmt, SQLSetStmtAttr, hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
                      reinterpret_cast<SQLPOINTER>(ROW_ARRAY_SIZE), 0);
    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROW_STATUS_PTR)", hstmt, SQLSetStmtAttr, hstmt, SQL_ATTR_ROW_STATUS_PTR,
                      row_array_status, 0);
    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROWS_FETCHED_PTR)", hstmt, SQLSetStmtAttr, hstmt,
                      SQL_ATTR_ROWS_FETCHED_PTR, &rows_fetched, 0);

    EXECUTE_AND_CHECK("SQLBindCol (TIMESTAMP)", hstmt, SQLBindCol, hstmt, 1, SQL_C_TYPE_TIMESTAMP, ts_array,
                      sizeof(ts_array[0]), ts_ind);

    EXECUTE_AND_CHECK("SQLExecDirect (SELECT)", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("SELECT * FROM ts_col_test ORDER BY col1"), SQL_NTS);
    EXECUTE_AND_CHECK("SQLFetchScroll", hstmt, SQLFetchScroll, hstmt, SQL_FETCH_NEXT, 0);

    REQUIRE(rows_fetched == ROW_ARRAY_SIZE);
    for (int i = 0; i < (int)rows_fetched; ++i) {
        if (row_array_status[i] == SQL_ROW_SUCCESS || row_array_status[i] == SQL_ROW_SUCCESS_WITH_INFO) {
            REQUIRE(ts_ind[i] != SQL_NO_DATA);
            REQUIRE(ts_array[i].year == 2000);
            REQUIRE(ts_array[i].month == 1);
            REQUIRE(ts_array[i].day == 1);
            REQUIRE(ts_array[i].second == i);
        }
    }

    EXECUTE_AND_CHECK("SQLFreeStmt (HSTMT)", hstmt, SQLFreeStmt, hstmt, SQL_CLOSE);
    EXECUTE_AND_CHECK("SQLFreeHandle (HSTMT)", hstmt, SQLFreeHandle, SQL_HANDLE_STMT, hstmt);
    DISCONNECT_FROM_DATABASE(env, dbc);
}

TEST_CASE("Test column-wise fetch of DATE array binding", "[odbc]") {
    SQLHANDLE env;
    SQLHANDLE dbc;
    HSTMT hstmt = SQL_NULL_HSTMT;

    CONNECT_TO_DATABASE(env, dbc);
    EXECUTE_AND_CHECK("SQLAllocHandle (HSTMT)", hstmt, SQLAllocHandle, SQL_HANDLE_STMT, dbc, &hstmt);

    EXECUTE_AND_CHECK("SQLExecDirect", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("DROP TABLE IF EXISTS date_col_test"), SQL_NTS);
    EXECUTE_AND_CHECK("SQLExecDirect", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("CREATE TABLE date_col_test (col1 DATE)"), SQL_NTS);

    for (int i = 0; i < ROW_ARRAY_SIZE; ++i) {
        char buf[128];
        snprintf(buf, sizeof(buf), "INSERT INTO date_col_test VALUES ('2000-01-%02d')", i + 1);
        EXECUTE_AND_CHECK("SQLExecDirect (INSERT)", hstmt, SQLExecDirect, hstmt, ConvertToSQLCHAR(buf), SQL_NTS);
    }

    SQL_DATE_STRUCT date_array[ROW_ARRAY_SIZE];
    SQLLEN date_ind[ROW_ARRAY_SIZE];
    SQLUSMALLINT row_array_status[ROW_ARRAY_SIZE];
    SQLULEN rows_fetched = 0;

    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROW_ARRAY_SIZE)", hstmt, SQLSetStmtAttr, hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
                      reinterpret_cast<SQLPOINTER>(ROW_ARRAY_SIZE), 0);
    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROW_STATUS_PTR)", hstmt, SQLSetStmtAttr, hstmt, SQL_ATTR_ROW_STATUS_PTR,
                      row_array_status, 0);
    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROWS_FETCHED_PTR)", hstmt, SQLSetStmtAttr, hstmt,
                      SQL_ATTR_ROWS_FETCHED_PTR, &rows_fetched, 0);

    EXECUTE_AND_CHECK("SQLBindCol (DATE)", hstmt, SQLBindCol, hstmt, 1, SQL_C_TYPE_DATE, date_array,
                      sizeof(date_array[0]), date_ind);

    EXECUTE_AND_CHECK("SQLExecDirect (SELECT)", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("SELECT * FROM date_col_test ORDER BY col1"), SQL_NTS);
    EXECUTE_AND_CHECK("SQLFetchScroll", hstmt, SQLFetchScroll, hstmt, SQL_FETCH_NEXT, 0);

    REQUIRE(rows_fetched == ROW_ARRAY_SIZE);
    for (int i = 0; i < (int)rows_fetched; ++i) {
        if (row_array_status[i] == SQL_ROW_SUCCESS || row_array_status[i] == SQL_ROW_SUCCESS_WITH_INFO) {
            REQUIRE(date_ind[i] != SQL_NO_DATA);
            REQUIRE(date_array[i].year == 2000);
            REQUIRE(date_array[i].month == 1);
            REQUIRE(date_array[i].day == i + 1);
        }
    }

    EXECUTE_AND_CHECK("SQLFreeStmt (HSTMT)", hstmt, SQLFreeStmt, hstmt, SQL_CLOSE);
    EXECUTE_AND_CHECK("SQLFreeHandle (HSTMT)", hstmt, SQLFreeHandle, SQL_HANDLE_STMT, hstmt);
    DISCONNECT_FROM_DATABASE(env, dbc);
}

TEST_CASE("Test column-wise fetch of TIME array binding", "[odbc]") {
    SQLHANDLE env;
    SQLHANDLE dbc;
    HSTMT hstmt = SQL_NULL_HSTMT;

    CONNECT_TO_DATABASE(env, dbc);
    EXECUTE_AND_CHECK("SQLAllocHandle (HSTMT)", hstmt, SQLAllocHandle, SQL_HANDLE_STMT, dbc, &hstmt);

    EXECUTE_AND_CHECK("SQLExecDirect", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("DROP TABLE IF EXISTS time_col_test"), SQL_NTS);
    EXECUTE_AND_CHECK("SQLExecDirect", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("CREATE TABLE time_col_test (col1 TIME)"), SQL_NTS);

    for (int i = 0; i < ROW_ARRAY_SIZE; ++i) {
        char buf[128];
        snprintf(buf, sizeof(buf), "INSERT INTO time_col_test VALUES ('0%d:0%d:0%d')", i, i, i);
        EXECUTE_AND_CHECK("SQLExecDirect (INSERT)", hstmt, SQLExecDirect, hstmt, ConvertToSQLCHAR(buf), SQL_NTS);
    }

    SQL_TIME_STRUCT time_array[ROW_ARRAY_SIZE];
    SQLLEN time_ind[ROW_ARRAY_SIZE];
    SQLUSMALLINT row_array_status[ROW_ARRAY_SIZE];
    SQLULEN rows_fetched = 0;

    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROW_ARRAY_SIZE)", hstmt, SQLSetStmtAttr, hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
                      reinterpret_cast<SQLPOINTER>(ROW_ARRAY_SIZE), 0);
    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROW_STATUS_PTR)", hstmt, SQLSetStmtAttr, hstmt, SQL_ATTR_ROW_STATUS_PTR,
                      row_array_status, 0);
    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROWS_FETCHED_PTR)", hstmt, SQLSetStmtAttr, hstmt,
                      SQL_ATTR_ROWS_FETCHED_PTR, &rows_fetched, 0);

    EXECUTE_AND_CHECK("SQLBindCol (TIME)", hstmt, SQLBindCol, hstmt, 1, SQL_C_TYPE_TIME, time_array,
                      sizeof(time_array[0]), time_ind);

    EXECUTE_AND_CHECK("SQLExecDirect (SELECT)", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("SELECT * FROM time_col_test ORDER BY col1"), SQL_NTS);
    EXECUTE_AND_CHECK("SQLFetchScroll", hstmt, SQLFetchScroll, hstmt, SQL_FETCH_NEXT, 0);

    REQUIRE(rows_fetched == ROW_ARRAY_SIZE);
    for (int i = 0; i < (int)rows_fetched; ++i) {
        if (row_array_status[i] == SQL_ROW_SUCCESS || row_array_status[i] == SQL_ROW_SUCCESS_WITH_INFO) {
            REQUIRE(time_ind[i] != SQL_NO_DATA);
            REQUIRE(time_array[i].hour == i);
            REQUIRE(time_array[i].minute == i);
            REQUIRE(time_array[i].second == i);
        }
    }

    EXECUTE_AND_CHECK("SQLFreeStmt (HSTMT)", hstmt, SQLFreeStmt, hstmt, SQL_CLOSE);
    EXECUTE_AND_CHECK("SQLFreeHandle (HSTMT)", hstmt, SQLFreeHandle, SQL_HANDLE_STMT, hstmt);
    DISCONNECT_FROM_DATABASE(env, dbc);
}

TEST_CASE("Test column-wise fetch of INTERVAL MONTH array binding", "[odbc]") {
    SQLHANDLE env;
    SQLHANDLE dbc;
    HSTMT hstmt = SQL_NULL_HSTMT;

    CONNECT_TO_DATABASE(env, dbc);
    EXECUTE_AND_CHECK("SQLAllocHandle (HSTMT)", hstmt, SQLAllocHandle, SQL_HANDLE_STMT, dbc, &hstmt);

    EXECUTE_AND_CHECK("SQLExecDirect", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("DROP TABLE IF EXISTS interval_col_test"), SQL_NTS);
    EXECUTE_AND_CHECK("SQLExecDirect", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("CREATE TABLE interval_col_test (col1 INTERVAL)"), SQL_NTS);

    const int ROWS = 12;
    SQL_INTERVAL_STRUCT interval_array[ROWS];
    SQLLEN interval_ind[ROWS];
    SQLUSMALLINT row_array_status[ROWS];
    SQLULEN rows_fetched = 0;

    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROW_ARRAY_SIZE)", hstmt, SQLSetStmtAttr, hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
                      reinterpret_cast<SQLPOINTER>(ROWS), 0);
    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROW_STATUS_PTR)", hstmt, SQLSetStmtAttr, hstmt, SQL_ATTR_ROW_STATUS_PTR,
                      row_array_status, 0);
    EXECUTE_AND_CHECK("SQLSetStmtAttr (SQL_ATTR_ROWS_FETCHED_PTR)", hstmt, SQLSetStmtAttr, hstmt,
                      SQL_ATTR_ROWS_FETCHED_PTR, &rows_fetched, 0);

    // bind as SQL_C_INTERVAL_MONTH (driver will set interval_type)
    EXECUTE_AND_CHECK("SQLBindCol (INTERVAL)", hstmt, SQLBindCol, hstmt, 1, SQL_C_INTERVAL_MONTH, interval_array,
                      sizeof(interval_array[0]), interval_ind);

    // select generated intervals without inserting data
    EXECUTE_AND_CHECK("SQLExecDirect (SELECT)", hstmt, SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("SELECT INTERVAL (i) MONTH FROM range(12) t(i)"), SQL_NTS);
    EXECUTE_AND_CHECK("SQLFetchScroll", hstmt, SQLFetchScroll, hstmt, SQL_FETCH_NEXT, 0);

    REQUIRE(rows_fetched == ROWS);
    for (int i = 0; i < ROWS; ++i) {
        if (row_array_status[i] == SQL_ROW_SUCCESS || row_array_status[i] == SQL_ROW_SUCCESS_WITH_INFO) {
            REQUIRE(interval_ind[i] != SQL_NO_DATA);
            // interval_type for MONTH should be SQL_IS_MONTH
            REQUIRE(interval_array[i].interval_type == SQL_IS_MONTH);
            // the month value should equal i
            REQUIRE(interval_array[i].intval.year_month.month == i);
        }
    }

    EXECUTE_AND_CHECK("SQLFreeStmt (HSTMT)", hstmt, SQLFreeStmt, hstmt, SQL_CLOSE);
    EXECUTE_AND_CHECK("SQLFreeHandle (HSTMT)", hstmt, SQLFreeHandle, SQL_HANDLE_STMT, hstmt);
    DISCONNECT_FROM_DATABASE(env, dbc);
}
