#include <float.h>
#include <math.h>

#define LERP(a, b, c) (((b) - (a)) * (c) + (a))

typedef struct UVT
{
  double u;
  double v;
  double t;
} UVT;

double const rt[31] = {/* reciprocal temperature (K) */
                 DBL_MIN, 10.0e-6, 20.0e-6, 30.0e-6, 40.0e-6, 50.0e-6,
                 60.0e-6, 70.0e-6, 80.0e-6, 90.0e-6, 100.0e-6, 125.0e-6,
                 150.0e-6, 175.0e-6, 200.0e-6, 225.0e-6, 250.0e-6, 275.0e-6,
                 300.0e-6, 325.0e-6, 350.0e-6, 375.0e-6, 400.0e-6, 425.0e-6,
                 450.0e-6, 475.0e-6, 500.0e-6, 525.0e-6, 550.0e-6, 575.0e-6,
                 600.0e-6};

UVT const uvt[31] = {
    {0.18006, 0.26352, -0.24341},
    {0.18066, 0.26589, -0.25479},
    {0.18133, 0.26846, -0.26876},
    {0.18208, 0.27119, -0.28539},
    {0.18293, 0.27407, -0.30470},
    {0.18388, 0.27709, -0.32675},
    {0.18494, 0.28021, -0.35156},
    {0.18611, 0.28342, -0.37915},
    {0.18740, 0.28668, -0.40955},
    {0.18880, 0.28997, -0.44278},
    {0.19032, 0.29326, -0.47888},
    {0.19462, 0.30141, -0.58204},
    {0.19962, 0.30921, -0.70471},
    {0.20525, 0.31647, -0.84901},
    {0.21142, 0.32312, -1.0182},
    {0.21807, 0.32909, -1.2168},
    {0.22511, 0.33439, -1.4512},
    {0.23247, 0.33904, -1.7298},
    {0.24010, 0.34308, -2.0637},
    {0.24792, 0.34655, -2.4681}, /* Note: 0.24792 is a corrected value for the error found in W&S as 0.24702 */
    {0.25591, 0.34951, -2.9641},
    {0.26400, 0.35200, -3.5814},
    {0.27218, 0.35407, -4.3633},
    {0.28039, 0.35577, -5.3762},
    {0.28863, 0.35714, -6.7262},
    {0.29685, 0.35823, -8.5955},
    {0.30505, 0.35907, -11.324},
    {0.31320, 0.35968, -15.628},
    {0.32129, 0.36011, -23.325},
    {0.32931, 0.36038, -40.770},
    {0.33724, 0.36051, -116.45}};

/// XYT to ColorTemp corelation copyright Bruce Lindbloom
/// http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_T.html
float XYZtoColorTemp(float x, float y, float z)
{
  float us, vs, p, di, dm;
  int i;

  if ((x < 1.0e-20) && (y < 1.0e-20) && (z < 1.0e-20))
    return 0.0; /* protect against possible divide-by-zero failure */
  us = (4.0 * x) / (x + 15.0 * y + 3.0 * z);
  vs = (6.0 * y) / (x + 15.0 * y + 3.0 * z);
  dm = 0.0;
  for (i = 0; i < 31; i++)
  {
    di = (vs - uvt[i].v) - uvt[i].t * (us - uvt[i].u);
    if ((i > 0) && (((di < 0.0) && (dm >= 0.0)) || ((di >= 0.0) && (dm < 0.0))))
      break; /* found lines bounding (us, vs) : i-1 and i */
    dm = di;
  }
  if (i == 31)
    return 0.0; /* bad XYZ input, color temp would be less than minimum of 1666.7 degrees, or too far towards blue */
  di = di / sqrt(1.0 + uvt[i].t * uvt[i].t);
  dm = dm / sqrt(1.0 + uvt[i - 1].t * uvt[i - 1].t);
  p = dm / (dm - di); /* p = interpolation parameter, 0.0 : i-1, 1.0 : i */
  p = 1.0 / (LERP(rt[i - 1], rt[i], p));
  return p;
}


void HSVtoRGB(float &fR, float &fG, float &fB, float &fH, float &fS, float &fV)
{
  float fC = fV * fS; // Chroma
  float fHPrime = fmod(fH / 60.0, 6);
  float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
  float fM = fV - fC;

  if (0 <= fHPrime && fHPrime < 1)
  {
    fR = fC;
    fG = fX;
    fB = 0;
  }
  else if (1 <= fHPrime && fHPrime < 2)
  {
    fR = fX;
    fG = fC;
    fB = 0;
  }
  else if (2 <= fHPrime && fHPrime < 3)
  {
    fR = 0;
    fG = fC;
    fB = fX;
  }
  else if (3 <= fHPrime && fHPrime < 4)
  {
    fR = 0;
    fG = fX;
    fB = fC;
  }
  else if (4 <= fHPrime && fHPrime < 5)
  {
    fR = fX;
    fG = 0;
    fB = fC;
  }
  else if (5 <= fHPrime && fHPrime < 6)
  {
    fR = fC;
    fG = 0;
    fB = fX;
  }
  else
  {
    fR = 0;
    fG = 0;
    fB = 0;
  }

  fR += fM;
  fG += fM;
  fB += fM;
}
static void RGBtoHSV(float &fR, float &fG, float fB, float &fH, float &fS, float &fV)
{
  float fCMax = max(max(fR, fG), fB);
  float fCMin = min(min(fR, fG), fB);
  float fDelta = fCMax - fCMin;
  if (fDelta > 0)
  {
    if (fCMax == fR)
    {
      fH = 60 * (fmod(((fG - fB) / fDelta), 6));
    }
    else if (fCMax == fG)
    {
      fH = 60 * (((fB - fR) / fDelta) + 2);
    }
    else if (fCMax == fB)
    {
      fH = 60 * (((fR - fG) / fDelta) + 4);
    }
    if (fCMax > 0)
    {
      fS = fDelta / fCMax;
    }
    else
    {
      fS = 0;
    }

    fV = fCMax;
  }
  else
  {
    fH = 0;
    fS = 0;
    fV = fCMax;
  }

  if (fH < 0)
  {
    fH = 360 + fH;
  }
}
