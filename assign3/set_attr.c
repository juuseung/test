#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dberror.h"
#include "expr.h"
#include "tables.h"

void setFloat(Schema *schema, int attrNum, Value *value, char *ptr) {
	*(float *) ptr = value->v.floatV;
	ptr += sizeof(float); // reset the current location of the data array by adding the size of typeLength of the attribute
}

void setBool(Schema *schema, int attrNum, Value *value, char *ptr) {
	*(bool *) ptr = value->v.boolV;
	ptr += sizeof(bool); // reset the current location of the data array by adding the size of typeLength of the attribute
}

void setString(Schema *schema, int attrNum, Value *value, char *ptr) {
	int size = schema->typeLength[attrNum];
	strncpy(ptr, value->v.stringV, size); // copy string from variable with amount of the typeLength to data
	ptr += schema->typeLength[attrNum]; // reset the current location of the data array by adding the size of typeLength of the attribute
}

void setInt(Schema *schema, int attrNum, Value *value, char *ptr) {
	*(int *) ptr = value->v.intV;
	ptr += sizeof(int); // reset the current location of the data array by adding the size of typeLength of the attribute
}
