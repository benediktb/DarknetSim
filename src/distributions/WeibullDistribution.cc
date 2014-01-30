/*
 * WeibullDistribution.cc
 */

#include "WeibullDistribution.h"

double WeibullDistribution::getNext() {
    return weibull(a, b);
}

