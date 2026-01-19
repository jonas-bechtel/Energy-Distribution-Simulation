#pragma once
#define FMT_UNICODE 0
#define FMT_HEADER_ONLY
#include <fmt/format.h>

struct GeneralParameter;

namespace General
{
	GeneralParameter GetParameters();
	void SetParameters(const GeneralParameter& params);
	void ShowParameterControls();
}

struct Parameter
{
	virtual std::string ToString() const = 0;
	virtual void FromString(std::string& str) = 0;
	virtual bool ShowControls() = 0;

	virtual ~Parameter() = default;
};

struct GeneralParameter : public Parameter
{
	// limit z range for energy dist generation and cooling force calculation
	bool limitZRange = false;
	float limitedZRange[2] = { -0.4f, 0.4f };

	std::string ToString() const override;
	void FromString(std::string& str) override;
	bool ShowControls() override;
};

struct MCMC_Parameter : public Parameter
{
	int numberSamples = (int)3e5;					
	int burnIn = 1000;								
	int lag = 30;									
	float proposalSigma[3] = {0.005f, 0.005f, 0.2f};
	int seed = (int)std::time(0);		

	bool changeSeed = true;
	bool automaticProposalStd = true;
	bool useInterpolation = false;
	bool generateAsync = true;

	std::string ToString() const override ;
	void FromString(std::string& str) override;
	bool ShowControls() override;
};

struct ElectronBeamParameter : public Parameter
{
	double detuningEnergy = 0.0;				
	double detuningVelocity = 0.0;				

	double transverse_kT = 2.0e-3;				
	double longitudinal_kT_estimate = 0.0;		
	double coolingEnergy = 0.15263;				
	double cathodeRadius = 0.0012955;			
	double expansionFactor = 30.0;				

	double electronCurrent = 1.2e-08;			
	double cathodeTemperature = 300.0;			
	double LLR = 1.9; 

	double sigmaLabEnergy = 0.0;				
	double extractionEnergy = 31.26;			

	std::string densityFile = "density file";	

	// optional analytic beam shapes
	bool gaussianElectronBeam = false;
	bool cylindricalElectronBeam = false;
	bool noElectronBeamBend = false;
	double electronBeamRadius = 0.05;
	double electronBeamDensity = 1;
	bool fixedLongitudinalTemperature = false;

	// parameters to increase histogram resolution by interpolation
	bool increaseHist = false;
	int factor = 3;

	bool mirrorAroundZ = true;
	bool cutOutZeros = true;

	std::string ToString() const override;
	void FromString(std::string& str) override;
	bool ShowControls() override;
};

struct LabEnergyParameter : public Parameter
{
	double centerLabEnergy = 0.0;
	double driftTubeVoltage = 0.0;
	std::string energyFile = "lab energy file";

	bool uniformLabEnergies = false;
	bool noSpaceCharge = false;
	bool interpolateEnergy = true;

	std::string ToString() const override;
	void FromString(std::string& str) override;
	bool ShowControls() override;
};

struct IonBeamParameter : public Parameter
{
	float shift[2] = { 0.0f, 0.0f };	
	float angles[2] = { 0.0f, 0.0f };	

	// always one gaussian
	float sigma[2] = { 0.01044f, 0.00455f };

	// optional second gaussian
	bool doubleGaussian = false;
	double amplitude = 10.1;			
	double amplitude2 = 1.0;
	float sigma2[2] = { 0.001f, 0.001f };

	std::string ToString() const override;
	void FromString(std::string& str) override;
	bool ShowControls() override;
};


struct OutputParameter : public Parameter
{
	float fitRange[2] = { 0.0f, 1.0f };		
	double fitDetuningEnergy = 1.0;			
	double fitLongitudinalTemperature = 0.0005;
	double fitTransverseTemperature = 0.002;
	double fitFWHM = 0.0;					
	double fitScalingFactor = 0.0;			
	double effectiveLength = 0.0;			

	double FWHM = 0.0;						
	double mainPeakPosition = 0.0;			
	float distancesFWHM[2] = { 0.0f, 0.0f };

	std::string ToString() const override;
	void FromString(std::string& str) override;
	bool ShowControls() override;
};

