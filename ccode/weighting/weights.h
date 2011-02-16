#ifndef _WEIGHTS_H
#define _WEIGHTS_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include <stdint.h>

#include "Catalog.h"

using namespace std;

void get_train_weights(struct Catalog& spec, struct Catalog& photo, int n_near);

#endif
