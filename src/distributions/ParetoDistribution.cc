/*
 * ParetoDistribution.cc
 */

#include "ParetoDistribution.h"

double ParetoDistribution::getNext() {
    return pareto_shifted(a, b, c);
}

