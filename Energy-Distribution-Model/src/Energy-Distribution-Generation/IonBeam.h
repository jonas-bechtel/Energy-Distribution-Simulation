#pragma once

#include "Point3D.h"
#include "HeatMapData.h"
#include "Parameter.h"

namespace IonBeam
{
	IonBeamParameter GetParameters();
	void SetParameters(const IonBeamParameter& params);

	void Init();

	void CreateFromReference(TH3D* reference);
	void UpdateHistData();

	TVector3 GetDirection();
	TVector3 GetVelocity();
	double GetValue(double x, double y, double z);
	double GetVelocityMagnitude();
	float GetSigmaX();
	float GetSigmaY();
	TH3D* Get();
	std::string GetTags();

	void ShowWindow();
	void ShowPlots();
	void ShowParameterControls();
	void ShowCoolingForceParameterControls();
}
