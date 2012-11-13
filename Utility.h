/*
 * Utility.h
 *
 *  Created on: Oct 2, 2012
 *      Author: bpassuello
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include <string>
#include <iomanip>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include "cpl_conv.h"
#include "cpl_string.h"

#define DEBUG 1
#define VERBOSE 0

#ifndef NULL
#define NULL 0
#endif

enum QT_ERR{
    FILE_ERROR,
    WRITE_ERROR,
    READ_ERROR,
    TREE_ERROR,
    NO_ERROR
};

static int emptyValue=0;
static char* outformat = "HFA";

#endif /* UTILITY_H_ */
