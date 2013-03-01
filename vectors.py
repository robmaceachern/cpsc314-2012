import numpy as np


# returns a new normalized vector
def normalize(vector):
    print 'normalizing vector:'
    print vector
    print 'length of vector:'
    print np.linalg.norm(vector)
    print 'result:'
    result = vector / np.linalg.norm(vector)
    print result
    print ''
    return result

print '-------------------'

def dot(vector1, vector2):
    return np.dot(vector1, vector2)

# # part 1
# ne = np.array([-.89, -.45, 0, 0])
# nf = np.array([.16, -.99, 0, 0])
# sumNeNf = ne + nf
# normalize(sumNeNf)

# part 2
ka = np.array([.9, .5, .5])
ks = np.array([1, .5, 1])
kd = np.array([.2, .8, .3])
nshiny = 10

intensityAmbient = np.array([.1, .1, .2])
intensityDiffuse = intensitySpecular = intensityLight = np.array([1, 1, 1])

eyePoint = np.array([1,2,0,1])
lightPoint = np.array([1,1,0,1])

aPoint = np.array([2,6,0,1])
bPoint = np.array([3,4,0,1])
cPoint = np.array([9,6,0,1])
dPoint = np.array([6,5,0,1])

# points
aNormal = np.array([-1,0,0,0])
# bNormal - calculated below
cNormal = np.array([.97, .52, 0, 0])
# dNormal

# faces
eNormal = np.array([-.89, -.45, 0, 0])
fNormal = np.array([.16, -.99, 0, 0])

print 'calc bNormal. Interpolation of E and F normals. The answer to part 1'
bNormal = normalize(eNormal + fNormal)

# Part 2

print 'Ambient, diffuse, and specular intensities multiplied by material coefficients'
kaIntentityA = ka * intensityAmbient
kdIntensityD = kd * intensityDiffuse
ksIntensityS = ks * intensitySpecular
print kaIntentityA, kdIntensityD, ksIntensityS

print 'Ambient component for phong lighting'
print kaIntentityA

print 'The vector at b pointing to the light'
bLight = normalize(lightPoint - bPoint)

print 'bLight dot bNormal'
print bLight, bNormal
print dot(bLight, bNormal)
print 'kdIntensityD * bLight dot bNormal'
bPhongDiffuse = kdIntensityD * dot(bLight, bNormal)
print bPhongDiffuse

print 'calculate bR'
bR = (2 * bNormal * dot(bNormal, bLight)) - bLight
bR = normalize(bR)
print bR

print 'calculate bV'
bV = normalize(eyePoint - bPoint)

print 'Specular component for point B: ksIntensityS * ((bV dot bR) ** nshiny)'
bPhongSpecular = ksIntensityS * (dot(bV, bR) ** nshiny)
print bPhongSpecular

print 'Total phong lighting for point B'
bItotal = (kaIntentityA + bPhongDiffuse + bPhongSpecular)
print bItotal
print '********'
print ''

cLight = normalize(lightPoint - cPoint)

print 'Diffuse component for point C *** Note: it is negative! Setting to zero'
cPhongDiffuse = kdIntensityD * dot(cLight, cNormal)
cPhongDiffuse = np.array([0,0,0])
print cPhongDiffuse

print 'calculate cR'
cR = (2 * cNormal * dot(cNormal, cLight)) - cLight
cR = normalize(cR)
print cR

print 'calculate cV'
cV = normalize(eyePoint - cPoint)

print 'Specular component for point C. Setting to zero'
cPhongSpecular = ksIntensityS * dot(cV, cR)
cPhongSpecular = np.array([0,0,0])
print cPhongSpecular

print 'Total phong lighting for point C'
cItotal = kaIntentityA
print cItotal
print '********'
print ''

print 'Part #3: Gouraud. Point D:'
print kaIntentityA
print bPhongDiffuse * .5
print bPhongSpecular * .5
print kaIntentityA + (bPhongDiffuse * .5) + (bPhongSpecular * .5)

print 'Part #4: Phong shading. Point D'
dNormal = normalize(bNormal + cNormal)

print 'calculate dV'
dV = normalize(eyePoint - dPoint)

print 'calculate dLight'
dLight = normalize(lightPoint - dPoint)

print 'calculate dR'
dR = (2 * dNormal * dot(dNormal, dLight)) - dLight
dR = normalize(dR)

print 'Diffuse component for D'
dPhongShadingDiffuse = kdIntensityD * dot(dNormal, dLight)
print dPhongShadingDiffuse
print dot(dNormal, dLight)

