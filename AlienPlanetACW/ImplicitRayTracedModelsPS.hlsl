cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

cbuffer InverseViewConstantBuffer : register(b1)
{
	matrix inverseView;
}

cbuffer CameraConstantBuffer : register(b2)
{
	float3 cameraPosition;
	float padding;
}

cbuffer timeConstantBuffer : register(b3)
{
	float time;
	float3 padding2;
}

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float2 canvasXY : TEXCOORD0;
};

static float MIN_DIST = 1.0;
static float MAX_DIST = 100.0;
static float EPSILON = 0.0001;

static float4 lightColour = float4(1.0f, 1.0f, 1.0f, 1.0f);
static float3 lightPosition = float3(0.0f, 3.0f, 0.0f);

struct RayTraceSphere
{
	float3 centre;
	float radius;
	float4 colour;
	float Kd, Ks, Kr, shininess;
};

static RayTraceSphere objects[3] = {
	//Sphere1
	{-1.5, 1.5f, 0.9, 0.0075, 1.0, 0.0, 0.0, 1.0, 0.3, 0.5, 0.7, 40.0f},
	//Sphere2
	{-1.8, 1.3f, 1.0, 0.004, 0.0, 1.0, 0.0, 1.0, 0.5, 0.7, 0.4, 40.0f},
	//Sphere3
	{-1.6, 1.4f, 1.3, 0.01, 1.0, 0.0, 1.0, 1.0, 0.5, 5.0, 0.3, 40.0f}
};

struct RayTraceTetra
{
	float3 centre;
	float size;
	float4 colour;
	float Kd, Ks, Kr, shininess;
};

static RayTraceTetra objectss[1] = {
	{-1.2, 1.5f, 1.3, 0.05, 1.0, 0.0, 0.0, 1.0, 0.6, 0.2, 0.3, 40.0f}
};

struct Ray {
	float3 o; //origin
	float3 d; //direction

};

float  plane(float3 p, float3 origin, float3 normal) {
	return dot(p - origin, normal);
}

float TetrahedranIntersect(RayTraceTetra t, Ray ray, out bool hit)
{
	float dn = 1.0 / sqrt(3.0);

	//float3 v = s.centre - ray.o;
	//float A = dot(v, ray.d);

	float p = t.centre;
	float d = t.size;

	//The tetrahedran is the intersection of four planes:
	float sd1 = plane(p, float3(d, d, d), float3(-dn, dn, dn));
	float sd2 = plane(p, float3(d, -d, -d), float3(dn, -dn, dn));
	float sd3 = plane(p, float3(-d, d, -d), float3(dn, dn, -dn));
	float sd4 = plane(p, float3(-d, -d, d), float3(-dn, -dn, -dn));

	float mint = max(max(sd1, sd2), max(sd3, sd4));

	if (mint < 0.002)
	{
		hit = true;
	}
	else
	{
		hit = false;
	}

	//max intersects shapes
	return mint;
}

float SphereIntersect(RayTraceSphere s, Ray ray, out bool hit)
{
	float t;
	float3 v = s.centre - ray.o;
	float A = dot(v, ray.d);
	float B = dot(v, v) - A * A;

	float R = sqrt(s.radius);

	if (B > R*R)
	{
		hit = false;
		t = MAX_DIST;
	}
	else
	{
		float disc = sqrt(R*R - B);
		t = A - disc;
		if (t < 0.0)
		{
			hit = false;
		}
		else
		{
			hit = true;
		}
	}

	return t;
}

float3 SphereNormal(RayTraceSphere s, float3 pos)
{
	return normalize(pos - s.centre);
}

float3 TetraNormal(RayTraceTetra t, float3 pos)
{
	return normalize(pos - t.centre);
}

bool AnyHit(Ray ray)
{
	bool hit;
	float t;

	bool anyhit = false;
	for (int i = 0; i < 3; i++)
	{
		t = SphereIntersect(objects[i], ray, hit);
		if (hit)
		{
			anyhit = true;
		}
	}

	//t = TetrahedranIntersect(objectss[0], ray, hit);

	//if (hit)
	//{
	//	anyhit = true;
	//}

	return anyhit;
}

float3 NearestHit(Ray ray, out int hitobj, out bool anyhit, out float mint)
{
	bool hit = false;
	mint = MAX_DIST;
	hitobj = -1;
	anyhit = false;
	float t;

	for (int i = 0; i < 3; i++)
	{
		t = SphereIntersect(objects[i], ray, hit);
		if (hit)
		{
			if (t < mint)
			{
				hitobj = i;
				mint = t;
				anyhit = true;
			}
		}
	}

	//t = TetrahedranIntersect(objectss[0], ray, hit);

	//if (hit)
	//{
	//	if (t < mint)
	//	{
	//		hitobj = 3;
	//		mint = t;
	//		anyhit = true;
	//	}
	//}

	return ray.o + ray.d*mint;
}

float4 Phong(float3 n, float3 l, float3 v, float shininess, float4 diffuseColor, float4 specularColor)
{
	float NdotL = dot(n, l);
	float diff = saturate(NdotL);
	float3 r = reflect(l, n);
	float spec = pow(saturate(dot(v, r)), shininess) * (NdotL > 0.0);

	return diff * diffuseColor + spec * specularColor;
}


float4 Shade(float3 hitPos, float3 normal, Ray ray, int hitobj, float lightIntensity)
{
	float3 lightDir = normalize(lightPosition - hitPos);

	float4 diff = (float4)0.0f;
	float4 spec = (float4)0.0f;
	float shininess = 0.0f;

	//if (hitobj == 3)
	//{
	//	diff = objectss[0].colour * objectss[0].Kd;
	//	spec = objectss[0].colour * objectss[0].Ks;
	//	shininess = objectss[0].shininess;
	//}
	//else
	{
		diff = objects[hitobj].colour * objects[hitobj].Kd;
		spec = objects[hitobj].colour * objects[hitobj].Ks;
		shininess = objects[hitobj].shininess;
	}

	if (AnyHit(ray))
	{
		return lightColour * lightIntensity * Phong(normal, lightDir, ray.d, shininess, diff, spec);
	}
	else
	{
		return (float4)1.0f;
	}

}

float4 RayTracing(Ray ray)
{
	int hitobj;
	bool hit = false;
	float3 n;
	float4 c = (float4)0;
	float lightIntensity = 1.0;

	float mint = 0.0f;

	//Calculate nearest hit
	float3 i = NearestHit(ray, hitobj, hit, mint);

	for (int depth = 1; depth < 5; depth++)
	{
		if (hit)
		{
			//if (hitobj == 3)
			//{
			//	n = TetraNormal(objectss[0], i);
			//}
			//else
			{
				n = SphereNormal(objects[hitobj], i);
			}

			c += Shade(i, n, ray, hitobj, lightIntensity);

			//Shoot reflect ray
			lightIntensity *= objects[hitobj].Kr;
			ray.o = i;
			ray.d = reflect(ray.d, n);
			float mint2 = 0.0f;
			i = NearestHit(ray, hitobj, hit, mint2);
		}
		//else
		//{
		//	c += backgroundColor / depth / depth;
		//}
	}

	return float4(mint, c.xyz);
}

struct outputPS
{
	float4 colour : SV_TARGET;
	float depth : SV_DEPTH;
};

// A pass-through function for the (interpolated) color data.
outputPS main(PixelShaderInput input)
{
	outputPS output;

	float3 PixelPos = float3(input.canvasXY, -MIN_DIST);

	Ray eyeray;
	eyeray.o = mul(float4(float3(0.0f, 0.0f, 0.0f), 1.0f), inverseView);
	eyeray.d = normalize(mul(float4(PixelPos, 0.0f), inverseView));

	float4 distanceAndColour = RayTracing(eyeray);

	if (distanceAndColour.x > MAX_DIST - EPSILON)
	{
		discard;
		//output.colour = float4(0.0f, 0.0f, 0.0f, 0.0f);
		//return float4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	float3 surfacePoint = cameraPosition + distanceAndColour.x * eyeray.d;

	float4 pv = mul(float4(surfacePoint, 1.0f), view);
	pv = mul(pv, projection);
	output.depth = pv.z / pv.w;

	output.colour = float4(distanceAndColour.yzw, 1.0f);

	output.colour = float4(lerp(output.colour.xyz, float3(1.0f, 0.97255f, 0.86275f), 1.0 - exp(-0.0005*distanceAndColour.x*distanceAndColour.x*distanceAndColour.x)), 1.0f);

	return output;
}