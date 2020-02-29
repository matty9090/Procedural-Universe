#include "ProcUtils.hpp"

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