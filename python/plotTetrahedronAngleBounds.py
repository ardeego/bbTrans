from discretized4dSphere import *
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

#paper
mpl.rc('font',size=40) 
mpl.rc('lines',linewidth=3.)
figSize = (14, 5.5)
figSize = (14, 12)
figSize = (21, 7)

def ToRad(deg):
  return deg*np.pi/180.
def ToDeg(rad):
  return rad/np.pi*180.

s3 = S3Grid(0)
tetras = s3.GetTetrahedra(0)

lvls = 20
dotMaxPred = [-1*np.ones(1)]*lvls
dotMaxPred[0] = tetras[0].GeMinMaxVertexDotProduct()[1]
for lvl in range(1,lvls):
  dotMaxPred[lvl] = dotMaxPred[lvl-1]*2./(1.+dotMaxPred[lvl-1])

dotMaxPredSqrt = [-1*np.ones(1)]*lvls
dotMaxPredSqrt[0] = tetras[0].GeMinMaxVertexDotProduct()[1]
for lvl in range(1,lvls):
  dotMaxPredSqrt[lvl] = np.sqrt((dotMaxPredSqrt[lvl-1]+1.)/2.)

dotMaxPred2 = [-1*np.ones(1)]*lvls
dotMaxPred2[0] = tetras[0].GeMinMaxVertexDotProduct()[1]
for lvl in range(1,lvls):
  dotMaxPred2[lvl] = (3.*dotMaxPred2[lvl-1]+1.)/(2.*(1. +dotMaxPred2[lvl-1]))

dotMinPred = [-1*np.ones(1)]*lvls
dotMinPred[0] = tetras[0].GeMinMaxVertexDotProduct()[0]
for lvl in range(1,lvls):
  a = 1./np.sqrt(2.)
  b = a - 0.5
  dotMinPred[lvl] = ((1.+b)/(1.+a))*((1.+a*dotMinPred[lvl-1])/(1.+b*dotMinPred[lvl-1]))

dotMax = [-1.]*lvls
for i in range(600):
  tetra = tetras[np.random.randint(0,len(tetras),1)]
  dotMax[0] = max(dotMax[0], tetra.GeMinMaxVertexDotProduct()[1])
  for lvl in range(1,lvls):
    tetra = tetra.Subdivide()[np.random.randint(0,6,1)]
    dotMax[lvl] = max(dotMax[lvl], tetra.GeMinMaxVertexDotProduct()[1])

dotMin = [1.]*lvls
for i in range(600):
  tetra = tetras[np.random.randint(0,len(tetras),1)]
  dotMin[0] = min(dotMin[0], tetra.GeMinMaxVertexDotProduct()[0])
  for lvl in range(1,lvls):
    tetra = tetra.Subdivide()[np.random.randint(0,6,1)]
    dotMin[lvl] = min(dotMin[lvl], tetra.GeMinMaxVertexDotProduct()[0])

dotMaxPred = [ToDeg(np.arccos(dotMaxPred_i)) for dotMaxPred_i in dotMaxPred]
dotMaxPredSqrt = [ToDeg(np.arccos(dotMaxPredSqrt_i)) for dotMaxPredSqrt_i in dotMaxPredSqrt]
dotMaxPred2 = [ToDeg(np.arccos(dotMaxPred2_i)) for dotMaxPred2_i in dotMaxPred2]
dotMax = [ToDeg(np.arccos(dotMax_i)) for dotMax_i in dotMax]
dotMin = [ToDeg(np.arccos(dotMin_i)) for dotMin_i in dotMin]
dotMinPred = [ToDeg(np.arccos(dotMinPred_i)) for dotMinPred_i in dotMinPred]

fig = plt.figure(figsize = figSize, dpi = 80, facecolor="w",
    edgecolor="k")
plt.fill_between(np.arange(20), dotMinPred, dotMaxPred,
  facecolor="orange", alpha=0.3)
plt.plot(dotMaxPredSqrt,"-", color="orange", label="upper/lower bound")
plt.plot(dotMaxPred,'-', color="orange")
plt.plot(dotMax,"--", color="red", label="actual min")
plt.plot(dotMin,"--", color="green", label="actual max")
plt.xlabel("subdivision level of the tetrahedron")
plt.ylabel("$\gamma$ [deg]")
plt.legend()
plt.tight_layout(0.4)
plt.savefig("../subdivisionVsMinAngle_ActualAndBound.png", figure=fig)

fig = plt.figure(figsize = figSize, dpi = 80, facecolor="w",
    edgecolor="k")
plt.fill_between(np.arange(20), dotMinPred, dotMaxPred,
  facecolor="orange", alpha=0.3)
plt.plot(dotMaxPredSqrt,"-", color="orange", label="upper/lower bound")
plt.plot(dotMaxPred2, 'c--', label=r"$\frac{1+3\gamma}{2(1+\gamma)}$")
#plt.plot(dotMaxPredSqrt,'g--', label=r"$\sqrt{\frac{1+\gamma}{2}}$")
plt.plot(dotMaxPred,'-', color="orange")
#plt.plot(dotMaxPred,'-', color="orange", label=r"$\frac{2\gamma}{1+\gamma}$")
plt.plot(dotMax,"--", color="red", label="actual min")
plt.plot(dotMin,"--", color="green", label="actual max")
plt.xlabel("subdivision level of the tetrahedron")
plt.ylabel("$\gamma$ [deg]")
plt.legend()
plt.tight_layout(0.4)
plt.savefig("../subdivisionVsMinAngle_ActualAndBound_allBounds.png", figure=fig)
plt.show()
