//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package darknetsim;

simple DarknetChurnNode extends DarknetOfflineDetectionNode {
    parameters:
	    @class(DarknetChurnNode);
        @statistic[sigChurnOff](record=stats,vector?);
        @statistic[sigChurnOn](record=stats,vector?);
	    
		bool startState = default(true);

		// For distributions, see subfolder distributions/ and the factory there
		string onTimeDistribution = default("exponential");
		string offTimeDistribution = default("exponential");

		double onTimeDistributionExponentialMean = default(30 * 60);
        double onTimeDistributionUniformB = default(60 * 60);
        double onTimeDistributionPoissonLambda = default(1.2);
        double onTimeDistributionParetoA = default(1.2);
        double onTimeDistributionParetoB = default(10);
        double onTimeDistributionParetoC = default(0);
        double onTimeDistributionWeibullA = default(1.2);
        double onTimeDistributionWeibullB = default(10);
		double onTimeDistributionLognormalMean = default(30 * 60);
		double onTimeDistributionLognormalVariance = default(30 * 60);

		double offTimeDistributionExponentialMean = default(30 * 60);
        double offTimeDistributionUniformB = default(60 * 60);
        double offTimeDistributionPoissonLambda = default(1.2);
        double offTimeDistributionParetoA = default(1.2);
        double offTimeDistributionParetoB = default(10);
        double offTimeDistributionParetoC = default(0);
        double offTimeDistributionWeibullA = default(1.2);
        double offTimeDistributionWeibullB = default(10);
		double offTimeDistributionLognormalMean = default(30 * 60);
		double offTimeDistributionLognormalVariance = default(30 * 60);
}