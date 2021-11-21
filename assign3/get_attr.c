#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dberror.h"
#include "expr.h"
#include "tables.h"

// get location of attribute from schema by using attribute number
RC getOffsetLocation (Schema *schema, int attrNum, int *location) {

	*location = 1; // initialize variable
	//for (int i = 0; i < attrNum; i++) { // get location using for loop from the very first to attrNum
	int i=0;
	while(i<attrNum){ // get location using while loop from the very first to attrNum
		// when data type is DT_FLOAT add the flozt size to the variable
		if (schema->dataTypes[i] == DT_FLOAT) *location += sizeof(float);
		// when data type is DT_BOOL add the boolean size to the variable
		else if (schema->dataTypes[i] == DT_BOOL) *location += sizeof(bool);
		// when data type is DT_STRING add the total length of the string to the variable
		else if (schema->dataTypes[i] == DT_STRING) *location += schema->typeLength[i];
		// when data type is DT_INT add the int size to the variable
		else if (schema->dataTypes[i] == DT_INT) *location += sizeof(int);
		
		i++;
	}
	//}

	return RC_OK;
}

// setAttr is to set attributes to the record by using attribute number based on the data type
char* getDataPointer(Record *record, Schema *schema, int attrNum) {
	int locationOffset = 0;
	getOffsetLocation(schema, attrNum, &locationOffset); // get location of attribute from given schema by using given attribute number
	char* ptr = record->data; // get the data array from record
	return ptr += locationOffset; // set the current location of the data array
}

void* getAttrFloat(Schema *schema, int attrNum, Value *attr, char *ptr) {
	float temp;
	memcpy(&temp, ptr, sizeof(float)); // copy memory from data with amount of the float size to variable
	attr->dt = DT_FLOAT, attr->v.floatV = temp; // set variable as copied memory and set data type as DT_FLOAT
}

void* getAttrBool(Schema *schema, int attrNum, Value *attr, char *ptr) {
	bool temp;
	memcpy(&temp, ptr, sizeof(bool)); // copy memory from data with amount of the boolean size to variable
	attr->dt = DT_BOOL, attr->v.boolV = temp; // set variable as copied memory and the data type as DT_BOOL
}

void* getAttrString(Schema *schema, int attrNum, Value *attr, char *ptr) {
	int size = schema->typeLength[attrNum] + 1;
	attr->v.stringV = (char *) malloc(size); // malloc a char structure
	strncpy(attr->v.stringV, ptr, size-1); // copy string from data with amount of the typeLength to variable
	attr->dt = DT_STRING, attr->v.stringV[size-1] = '\0'; // add null value to the end of variable and set data type as DT_STRING
}

void* getAttrInt(Schema *schema, int attrNum, Value *attr, char *ptr) {
	int temp = 0; // initialize variable
	memcpy(&temp, ptr, sizeof(int)); // copy memory from data with amount of the int size to variable
	attr->dt = DT_INT, attr->v.intV = temp; // set variable as copied memory and set data type as DT_INT
}
