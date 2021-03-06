#pragma once

#include <random>
#include <string>

#include <d3d11.h>
#include <SimpleMath.h>

namespace ProcUtils
{
	void RGBToHSL(DirectX::SimpleMath::Color rgb, int& H, int& S, int& L);
	void HSLToRGB(float H, float S, float L, DirectX::SimpleMath::Color& rgb);

	std::string RandomGalaxyName(std::default_random_engine& gen);
	std::string RandomStarName(std::default_random_engine& gen);
	DirectX::SimpleMath::Color RandomColour(std::default_random_engine& gen);
}