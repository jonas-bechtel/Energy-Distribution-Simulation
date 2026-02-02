#pragma once
class CrossSection;

namespace BoltzmannDistribution
{
	double Function(double energy, double temperature);

	void Update(const CrossSection& currentCS);
	void Plot(bool showMarkers);

	void ShowWindow(bool& show, const CrossSection& currentCS);
}

