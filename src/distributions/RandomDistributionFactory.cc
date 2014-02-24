/*
 * RandomDistributionFactory.cc
 */

#include <cmodule.h>
#include "RandomDistributionFactory.h"
#include "ExponentialDistribution.h"
#include "UniformDistribution.h"
#include "PoissonDistribution.h"
#include "ParetoDistribution.h"
#include "WeibullDistribution.h"
#include "LognormalDistribution.h"

IRandomDistribution* RandomDistributionFactory::getDistribution(std::string distributionName, cModule* module, std::string parPrefix) {
    if (distributionName == "exponential") {
        double mean = module->par((parPrefix + "DistributionExponentialMean").c_str()).doubleValue();
        return new ExponentialDistribution(mean);
    } else if (distributionName == "uniform") {
        double b = module->par((parPrefix + "DistributionUniformB").c_str()).doubleValue();
        return new UniformDistribution(b);
    } else if (distributionName == "poisson") {
        double lambda = module->par((parPrefix + "DistributionPoissonLambda").c_str()).doubleValue();
        return new PoissonDistribution(lambda);
    } else if (distributionName == "pareto") {
        double a = module->par((parPrefix + "DistributionParetoA").c_str()).doubleValue();
        double b = module->par((parPrefix + "DistributionParetoB").c_str()).doubleValue();
        double c = module->par((parPrefix + "DistributionParetoC").c_str()).doubleValue();
        return new ParetoDistribution(a, b, c);
    } else if (distributionName == "weibull") {
        double a = module->par((parPrefix + "DistributionWeibullA").c_str()).doubleValue();
        double b = module->par((parPrefix + "DistributionWeibullB").c_str()).doubleValue();
        return new WeibullDistribution(a, b);
    } else if (distributionName == "lognormal") {
        double mean = module->par((parPrefix + "DistributionLognormalMean").c_str()).doubleValue();
        double variance = module->par((parPrefix + "DistributionLognormalVariance").c_str()).doubleValue();
        return new LognormalDistribution(mean, variance);
    }

    return (IRandomDistribution*) NULL;
}
