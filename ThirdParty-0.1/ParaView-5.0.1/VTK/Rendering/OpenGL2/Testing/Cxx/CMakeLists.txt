vtk_add_test_cxx(${vtk-module}CxxTests tests
  TestAppleBug.cxx
  TestDepthOfFieldPass.cxx
  TestLightingMapLuminancePass.cxx
  TestLightingMapNormalsPass.cxx
  TestPointGaussianMapper.cxx
  TestPointGaussianMapperOpacity.cxx
  TestSetZBuffer.cxx
  TestUserShader.cxx
  TestUserShader2.cxx
  TestValuePass.cxx
  TestValuePass2.cxx
  TestVBOPLYMapper.cxx
  TestVBOPointsLines.cxx
  TestGaussianBlurPass.cxx
  TestSSAAPass.cxx
  TestBlurAndSobelPasses.cxx
  TestShadowMapBakerPass.cxx
  TestShadowMapPass.cxx
  TestSobelGradientMagnitudePass.cxx
  TestEDLPass.cxx
  TestCoincident.cxx
  TestRenderToImage.cxx
  )

vtk_test_cxx_executable(${vtk-module}CxxTests tests RENDERING_FACTORY)
