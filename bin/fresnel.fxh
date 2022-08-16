float Fresnel(float cosI, float eta) 
{ 
	float c = cosI * eta;
	float g = sqrt(1 + c * c - eta * eta);
	float a = (g - c) / (g + c);
	float b = (c * (g + c) - eta * eta) 
			/ (c * (g - c) + eta * eta);
	float result = 0.5 * a * a * (1 + b * b);
    return result;
}

float Phase(float cosI, float g) 
{ 
	float g2 = g * g;
    return (1 - g2) / (4 * 3.1415926 * pow(1 + g2 - 2 * g * cosI, 1.5));
}