#pragma once
#include "CrossSection.h"
#include "RateCoefficient.h"
#include "PlasmaRateCoefficient.h"

namespace DeconvolutionWindow
{
	void Init();

	void AddRateCoefficientToList(RateCoefficient& rc);
	void RemoveRateCoefficient(int index);

	void AddCrossSectionToList(CrossSection& cs);
	void RemoveCrossSection(int index);

	void AddPlasmaRateToList(PlasmaRateCoefficient& prc);
	void RemovePlasmaRate(int index);

	RateCoefficient& GetCurrentRC();
	CrossSection& GetCurrentCS();
	PlasmaRateCoefficient& GetCurrentPRC();

	void ShowWindow();
	void ShowSettings();
	void ShowPlots();
	void ShowPlasmaRateWindow();

	void OnDeconvolveButtonClicked();
	void OnConvolveButtonClicked();

	void ShowSizeMismatchPopup();
	void ShowMissingDataPopup();

	void ShowRateCoefficientList();
	void ShowCrossSectionList();
	void ShowPlasmaRateList();
}
