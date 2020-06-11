#pragma once
#include <easyxmath.h>

class color {
public:
	int R;
	int G;
	int B;
	color(int r, int g, int b) {
		R = r; G = g; B = b;
	}
	color(float r, float g, float b) {
		R = (int)r; G = (int)g; B = (int)b;
	}
	color(unsigned long RGB = 0) {
		B = RGB % 0x100;
		RGB /= 0x100;
		G = RGB % 0x100;
		RGB /= 0x100;
		R = RGB % 0x100;
	}
	unsigned long getRGB() const {
		return R * 0x10000 + G * 0x100 + B;
	}
	unsigned long getBGR() const {
		return B * 0x10000 + G * 0x100 + R;
	}
	color operator*(const vec3 &vec) const {
		return color((int)(R * vec.x), (int)(G * vec.y), (int)(B * vec.z));
	}
	friend color operator*(const vec3 &vec, const color &Color) {
		return Color * vec;
	}
	color operator*(float coef) const {
		return color((int)(R * coef), (int)(G * coef), (int)(B * coef));
	}
	friend color operator*(float coef, const color &Color) {
		return Color * coef;
	}
	color operator+(const color &Color) const {
		return color(min(Color.R + R, 0xFF), min(Color.G + G, 0xFF), min(Color.B + B, 0xFF));
	}
	color &operator+=(const color &Color) {
		R = min(R + Color.R, 0xFF);
		G = min(G + Color.G, 0xFF);
		B = min(B + Color.B, 0xFF);
		return *this;
	}
};

class ray {
public:
	pos e;
	vec3 d;
	ray(const pos &point, const vec3 &dir) {
		e = point; d = dir;
	}
};

class surface {
public:
	virtual bool intersect_ray(const ray &line, float &t)const = 0;
	virtual vec3 normal(const pos &p)const = 0;
};

class sphere : virtual public surface{
public:
	pos c;
	float r;
	sphere(const pos &center, float radius) : c(center), r(radius) {}
	bool intersect_ray(const ray &line, float &t) const override;
	vec3 normal(const pos &p) const override;
};

class triangle : virtual public surface{
public:
	pos A;
	pos B;
	pos C;
	triangle(const pos &a, const pos &b, const pos &c):A(a), B(b), C(c) {}
	bool intersect_ray(const ray &r, float &t) const override;
	vec3 normal(const pos &p)const override;
};

class colored_surface : virtual public surface {
public:
	color rgb;
	colored_surface(const color &Color = color(0xFFFFFFul)):rgb(Color){}
};

class colored_sphere : public sphere, public colored_surface{
public:
	colored_sphere(const pos &center, float radius, const color &Color = color(0xFFFFFFul)) : colored_surface(Color), sphere(center, radius){}
	bool intersect_ray(const ray &line, float &t) const override {
		return sphere::intersect_ray(line, t);
	}
	vec3 normal(const pos &p) const override {
		return sphere::normal(p);
	}
};

class colored_triangle : public triangle, public colored_surface{
public:
	colored_triangle(const pos &a, const pos &b, const pos &c, const color &Color = color(0xFFFFFFul)): colored_surface(Color), triangle(a, b, c){}
	bool intersect_ray(const ray &r, float &t) const override {
		return triangle::intersect_ray(r, t);
	}
	vec3 normal(const pos &p)const override {
		return triangle::normal(p);
	}
};



bool sphere::intersect_ray(const ray &line, float &t) const{
	float A = dot(line.d, line.d);
	float B = 2 * dot(line.d, line.e - c);
	float C = dot(line.e - c, line.e - c) - r * r;
	float delta = B * B - 4 * A * C;
	if (delta >= 0) {
		t = (-B - sqrt(delta)) / (2 * A);
		if (t < 0) return false;
		return true;
	}
	return false;
}

inline vec3 sphere::normal(const pos &p) const {
	return (p - c).normalize();
}


bool triangle::intersect_ray(const ray &r, float &t) const{
	float a = A.x - B.x;
	float b = A.y - B.y;
	float c = A.z - B.z;
	float d = A.x - C.x;
	float e = A.y - C.y;
	float f = A.z - C.z;
	float g = r.d.x;
	float h = r.d.y;
	float i = r.d.z;
	float j = A.x - r.e.x;
	float k = A.y - r.e.y;
	float l = A.z - r.e.z;
	float t1 = e * i - h * f;
	float t2 = g * f - d * i;
	float t3 = d * h - e * g;
	float t4 = a * k - j * b;
	float t5 = j * c - a * l;
	float t6 = b * l - k * c;
	float M = a * t1 + b * t2 + c * t3;
	t = -(f * t4 + e * t5 + d * t6) / M;
	if (t < 0) return false;
	float gamma = (i * t4 + h * t5 + g * t6) / M;
	if (gamma < -0.0f || gamma > 1.0f) return false;
	float beta = (j * t1 + k * t2 + l * t3) / M;
	if (beta < -0.0f || beta > 1.0f - gamma) return false;
	return true;
}

inline vec3 triangle::normal(const pos &p)const {
	vec3 Cross = cross(B - A, C - A).normalize();
	vec3 test = p - A;
	if (dot(Cross, test) > 0) {
		return Cross;
	}
	else {
		return -Cross;
	}
}