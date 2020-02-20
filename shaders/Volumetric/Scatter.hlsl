/*
	Adapted from the book GPU Gems
	Author: Sean O'Neil
	Source: https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter16.html
*/

cbuffer ScatterBuffer : register(b1) {
	float3 v3CameraPos;
	float fCameraHeight;
	float3 v3LightPos;
	float fCameraHeight2;
	float3 v3InvWavelength;
	float fOuterRadius;
	float fOuterRadius2;
	float fInnerRadius;
	float fInnerRadius2;
	float fKrESun;
	float fKmESun;
	float fKr4PI;
	float fKm4PI;
	float fScale;
	float fScaleDepth;
	float fScaleOverScaleDepth;
	float g;
	float g2;
}

static float3 PrimaryColour;
static float3 SecondaryColour;
static float3 T0;
static float Depth;

// Calculates the Mie phase function
float getMiePhase(float fCos, float fCos2, float g, float g2) {
	return 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0 * g * fCos, 1.5);
}

// Calculates the Rayleigh phase function
float getRayleighPhase(float fCos2) {
	return 0.75 + 0.75 * fCos2;
}

float scale(float fCos) {
	float x = 1.0 - fCos;
	return fScaleDepth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}

void scatter(float3 pos) {
	// Get the ray from the camera to the vertex and its length (which
	// is the far point of the ray passing through the atmosphere)
	float3 v3Pos = pos;
	float3 v3Ray = v3Pos - v3CameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;
	
	// Calculate the closest intersection of the ray with
	// the outer atmosphere (point A in Figure 16-3)

	float b = 2.0 * dot(v3CameraPos, v3Ray);
	float c = fCameraHeight2 - fOuterRadius2;
	float det = max(0.0, b * b - 4.0 * c);
	float fNear = 0.5 * (-b - sqrt(det));

	// Calculate the ray's start and end positions in the atmosphere,
	// then calculate its scattering offset

	float3 v3Start = v3CameraPos + v3Ray * fNear;
	fFar -= fNear;
	float fStartAngle = dot(v3Ray, v3Start) / fOuterRadius;
	float fStartDepth = exp(-1.0 / fScaleDepth);
	float fStartOffset = fStartDepth * scale(fStartAngle);
	Depth = clamp(fStartOffset, 0.0, 1.0);

	const int fSamples = 2.0f;
	
	// Initialize the scattering loop variables
	float fSampleLength = fFar / fSamples;
	float fScaledLength = fSampleLength * fScale;
	
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

	// Now loop through the sample points

	float3 v3FrontColor = float3(0.0, 0.0, 0.0);

	for(int i = 0; i < fSamples; i++) {
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		float fScatter = (fStartOffset + fDepth * (scale(fLightAngle) - scale(fCameraAngle)));

		float3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));

		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}
	
	PrimaryColour = v3FrontColor * (v3InvWavelength * fKrESun);
	SecondaryColour = v3FrontColor * fKmESun;
	T0 = v3CameraPos - v3Pos;
}

void scatter_surf(float3 pos) {
	// Get the ray from the camera to the vertex and its length (which
	// is the far point of the ray passing through the atmosphere)
	
	float3 v3Pos = pos;
	float3 v3Ray = v3Pos - v3CameraPos;
	float fFar = length(v3Ray);

	v3Ray /= fFar;
	
	// Calculate the closest intersection of the ray with
	// the outer atmosphere (point A in Figure 16-3)

	float fNear = 0.0;
	
	if(fCameraHeight2 > fOuterRadius2) {
		float b = 2.0 * dot(v3CameraPos, v3Ray);
		float c = 4.0 * (fCameraHeight2 - fOuterRadius2);
		float det = max(0.0, b * b - c);
		fNear = 0.5 * (-b - sqrt(det));
	}

	// Calculate the ray's start and end positions in the atmosphere,
	// then calculate its scattering offset

	float3 v3Start = v3CameraPos + v3Ray * fNear;
	fFar -= fNear;
	
	float fDepth = exp((fInnerRadius - fOuterRadius) / fScaleDepth);
	float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);
	float fLightAngle = dot(v3LightPos, v3Pos) / length(v3Pos);
	float fCameraScale = scale(fCameraAngle);
	float fLightScale = scale(fLightAngle);
	float fCameraOffset = fDepth * fCameraScale;
	float fTemp = (fLightScale + fCameraScale);

	const float fSamples = 3.0f;
	
	// Initialize the scattering loop variables
	float fSampleLength = fFar / fSamples;
	float fScaledLength = fSampleLength * fScale;
	
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

	// Now loop through the sample points

	float3 v3FrontColor = float3(0.0, 0.0, 0.0);
	float3 v3Attenuate;

	for(int i = 0; i < (int)fSamples; i++) {
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fScatter = fDepth * fTemp - fCameraOffset;
		
		v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}
	
	PrimaryColour = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);
	SecondaryColour = v3Attenuate;
}

float3 getScatterColour(float3 c1, float3 c2, float3 t0) {
	float fCos = dot(v3LightPos, t0) / length(t0);
	float fCos2 = fCos * fCos;

	return getRayleighPhase(fCos2) * c1 + getMiePhase(fCos, fCos2, g, g2) * c2;
}