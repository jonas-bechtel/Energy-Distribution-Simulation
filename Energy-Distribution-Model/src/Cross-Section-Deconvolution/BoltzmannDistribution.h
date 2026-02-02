#pragma once
class CrossSection;

namespace BoltzmannDistribution
{
	double Function(double energy, double temperature);

	void Update(CrossSection& currentCS);
	void Plot(bool showMarkers);

	void ShowWindow(bool& show, CrossSection& currentCS);
}

