#ifndef __SIMPLE_JSON_H__
#define __SIMPLE_JSON_H__

typedef struct SJList_S SJList;
typedef struct SJString_S SJString;
typedef struct SJson_S SJson;

/**
 * @brief frees a previously loaded json struct
 */
void sj_free(SJson *sjs);

/**
 * @brief make a duplicate of a json structure.
 * @note the duplicate needs to be sj_free()'d separately
 * @param json the json to be duplicated
 * @return NULL on error, or a Duplicate json
 */
SJson *sj_copy(SJson *json);

/**
 * @brief loads and parses a json file
 * @param filename the file to parse
 * @return NULL on error or an SJS pointer
 */
SJson *sj_load(const char *filename);

/**
 * @brief write a json value as a formatted json string to file
 * @param json the struct to convert and write
 * @param filename the file to overwrite
 */
void sj_save(SJson *json,char *filename);

/**
 * @brief make a new json value that is a string
 * @param str the string to base the json value on
 * @return NULL on error or a new json that holds a string value
 */
SJson *sj_new_str(char *str);

/**
 * @brief get the JSON value as a string
 * @param json the json value to get the string from
 * @return NULL if the provided json was not a string or on error, the string otherwise
 * @note: the returned value should not be altered or freed
 */
const char *sj_get_string_value(SJson *json);

/**
 * @brief get the json value as an integer
 * Can be used to check for existence
 * @param json the json value to get from
 * @param i [optional] place to write the output to
 * @return 0 if the json did not contain an integer or if there was an error, 1 otherwise
 */
int sj_get_integer_value(SJson *json,int *i);

/**
 * @brief get the json value as a float
 * Can be used to check for existence
 * @param json the json value to get from
 * @param f [optional] place to write the output to
 * @return 0 if the json did not contain a float or if there was an error, 1 otherwise
 */
int sj_get_float_value(SJson *json,float *f);

/**
 * @brief get the json value as a bool
 * Can be used to check for existence
 * @param json the json value to get from
 * @param b [optional] place to write the output to
 * @return 0 if the json did not contain a bool or if there was an error, 1 otherwise
 */
int sj_get_bool_value(SJson *json,short int *b);

/**
 * @brief make a new json value that is an integer
 * @param i the integer value
 * @return NULL on error or a new json that holds an integer value
 */
SJson *sj_new_int(int i);

/**
 * @brief make a new json value that is a float
 * @param f the float value
 * @return NULL on error or a new json that holds a floating point value
 */
SJson *sj_new_float(float f);

/**
 * @brief make a new json value that is a bool
 * @param b the boolean value (1 is true, 0 is false)
 * @return NULL on error or a new json that holds a bool value
 */
SJson *sj_new_bool(int b);

/**
 * @brief make a new json value that is NULL
 * @return NULL on error (Ironically) or a new json that holds a NULL value
 */
SJson *sj_null_new();

/**
 * @brief allocate a new empty json object
 * @return NULL on memory allocation error, an empty SJson object otherwise
 */
SJson *sj_object_new();

/**
 * @brief insert data into a json object
 * @param object the json object to insert into
 * @param key the string to identify the object value with
 * @param value the value of the key
 * @note: this is a no-op if object is not an actual JSON object
 */
void sj_object_insert(SJson *object,char *key,SJson *value);

/**
 * @brief get the json value from an object given a key
 * @param object the hson object to get
 * @param key the key to search by
 * @return NULL on error, or if no object provided or it is not of object or the key is not found, the value otherwise
 * @note: the json returned is OWNED by the parent object do not free it.
 */
SJson *sj_object_get_value(SJson *object,char *key);

/**
 * @brief allocate a new empty json array
 * @return NULL on memory allocation error, an empty SJson array otherwise
 */
SJson *sj_array_new();

/**
 * @brief append to a JSON array a new value
 * @param array the JSON array to append to
 * @param value the value to add to the string
 * @note: this is a no-op if array is not an actual JSON array
 */
void sj_array_append(SJson *array,SJson *value);

/**
 * @brief get the number of elements in the json array
 * @param array the json array
 * @return 0 if there is an error, the count otherwise
 */
int sj_array_get_count(SJson *array);

/**
 * @brief retrieve the nth element in the json array
 * @param array the array to search through
 * @param n the index of the element to get
 * @return NULL on error (check sj_get_error()) or the SJson value otherwise
 */
SJson *sj_array_get_nth(SJson *array,int n);

/**
 * @brief print the contents of the json file to stdout
 * @param json the json struct to print
 */
void sj_echo(SJson *json);

/**
 * @brief check if the json is an array
 * @param json the json to check
 * @return 1 if it is, 0 if not
 */
int sj_is_array(SJson *json);

/**
 * @brief check if the json is an object
 * @param json the json to check
 * @return 1 if it is, 0 if not
 */
int sj_is_object(SJson *json);

/**
 * @brief check if the json is a string
 * @param json the json to check
 * @return 1 if it is, 0 if not
 */
int sj_is_string(SJson *json);

/**
 * @brief check if the json is a number
 * @param json the json to check
 * @return 1 if it is, 0 if not
 */
int sj_is_number(SJson *json);

/**
 * @brief check if the json is a bool
 * @param json the json to check
 * @return 1 if it is, 0 if not
 */
int sj_is_bool(SJson *json);

/**
 * @brief check if the json is NULL
 * @param json the json to check
 * @return 1 if it is, 0 if not
 */
int sj_is_null(SJson *json);

/**
 * @brief get the last reported error for simple json
 * @return a character array with the error message
*/
char *sj_get_error();

/**
 * @brief used internally to se the error message
 * @param error the error message to set
 */
void sj_set_error(char *error);

#endif
