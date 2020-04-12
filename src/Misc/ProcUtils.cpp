#include "ProcUtils.hpp"
#include <sstream>

float Min(float f1, float f2, float f3) {
	float fMin = f1;

	if (f2 < fMin)
		fMin = f2;

	if (f3 < fMin)
		fMin = f3;

	return fMin;
}

// Find the maximum of three numbers (helper function for exercise below)
float Max(float f1, float f2, float f3) {
	float fMax = f1;

	if (f2 > fMax)
		fMax = f2;

	if (f3 > fMax)
		fMax = f3;

	return fMax;
}

// Convert an RGB colour to a HSL colour
void ProcUtils::RGBToHSL(DirectX::SimpleMath::Color rgb, int& H, int& S, int& L) {
	float min = Min(rgb.R(), rgb.G(), rgb.B());
	float max = Max(rgb.R(), rgb.G(), rgb.B());

	L = static_cast<int>(50.f * (max + min));

	if (min == max) {
		S = H = 0;
		return;
	}

	if (L < 50)
		S = static_cast<int>(100.f * (max - min) / (max + min));
	else
		S = static_cast<int>(100.f * (max - min) / (2.f - max - min));

	if (max == rgb.R()) H = static_cast<int>(60.f * (rgb.G() - rgb.B()) / (max - min));
	if (max == rgb.G()) H = static_cast<int>(60.f * (rgb.B() - rgb.R()) / (max - min) + 120.f);
	if (max == rgb.B()) H = static_cast<int>(60.f * (rgb.R() - rgb.G()) / (max - min) + 240.f);

	if (H < 0) H += 360;
}

// Convert a HSL colour to an RGB colour
void ProcUtils::HSLToRGB(float H, float S, float L, DirectX::SimpleMath::Color& rgb) {
	S /= 100.f, L /= 100.f;

	float hh = H / 60.0f;
	float C = (1.0f - fabs(2.0f * L - 1.0f)) * S;
	float X = C * (1.0f - fabs(fmodf(hh, 2.0f) - 1.0f));
	float fR = 0.0f, fG = 0.0f, fB = 0.0f;

	if (hh >= 0 && hh < 1) { fR = C; fG = X; }
	else if (hh >= 1 && hh < 2) { fR = X; fG = C; }
	else if (hh >= 2 && hh < 3) { fG = C; fB = X; }
	else if (hh >= 3 && hh < 4) { fG = X; fB = C; }
	else if (hh >= 4 && hh < 5) { fR = X; fB = C; }
	else { fR = C; fB = X; }

	float m = L - C / 2.0f;

	rgb = DirectX::SimpleMath::Color(fR + m, fG + m, fB + m);
}

std::string ProcUtils::RandomGalaxyName(std::default_random_engine& gen)
{
	std::uniform_int_distribution<> numDist(100, 9999);
	std::uniform_int_distribution<> prefixDist(2, 3);
	std::uniform_int_distribution<> letterDist('A', 'Z');

	std::ostringstream name;

	for (int i = 0; i < prefixDist(gen); ++i)
		name << static_cast<char>(letterDist(gen));

	name << " " << numDist(gen);

	return name.str();
}

std::string ProcUtils::RandomStarName(std::default_random_engine& gen)
{
	std::uniform_int_distribution<> numDist(0, 2);
	std::uniform_int_distribution<> lenDist(4, 7);
	std::uniform_int_distribution<> boolDist(0, 1);
	std::uniform_int_distribution<> conDist(0, 20);
	std::uniform_int_distribution<> vowDist(0, 4);
	std::uniform_int_distribution<> idDist(1, 99);
	std::uniform_int_distribution<> letterDist('A', 'Z');

	std::string S = "BCDFGHJKLMNPQRSTVWXYZ";
	std::string L = "AEIOU";
	std::string C = "bcdfghjklmnpqrstvwxyz";
	std::string V = "aeiou";
	std::string name;

	bool sv = boolDist(gen);
	name += sv ? S[conDist(gen)] : L[vowDist(gen)];

	for (int i = 0; i < lenDist(gen); ++i) {
		sv = !sv;
		name += sv ? C[conDist(gen)] : V[vowDist(gen)];
	}

	switch (numDist(gen))
	{
		case 0: {
			std::ostringstream ss;
			ss << static_cast<char>(letterDist(gen));
			ss << static_cast<char>(letterDist(gen));
			name = ss.str() + " " + name;
		}
		break;

		case 1: {
			std::ostringstream ss;
			ss << static_cast<char>(letterDist(gen));
			ss << static_cast<char>(letterDist(gen));
			name = ss.str() + " " + name + " " + std::to_string(idDist(gen));
		}
		break;

		case 2: {
			name = name + " " + std::to_string(idDist(gen));
		}
		break;
	}

	return name;
}

DirectX::SimpleMath::Color ProcUtils::RandomColour(std::default_random_engine& gen)
{
	DirectX::SimpleMath::Color col;
	std::uniform_real_distribution<float> dist(0.0f, 360.0f);
	ProcUtils::HSLToRGB(dist(gen), 100.0f, 50.0f, col);

	return col;
}