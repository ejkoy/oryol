fips_begin_module(Assets)
    fips_vs_warning_level(3)
    fips_dir(Gfx)
    fips_files(
        MeshBuilder.cc MeshBuilder.h
        ShapeBuilder.cc ShapeBuilder.h
        VertexWriter.cc VertexWriter.h
        TextureLoader.cc TextureLoader.h
        OmshParser.cc OmshParser.h
        MeshLoader.cc MeshLoader.h
    )
    fips_dir(Sound)
    fips_files(
        SoundGen.cc SoundGen.h
    )
fips_end_module()

fips_begin_unittest(Assets)
    fips_vs_warning_level(3)
    fips_dir(UnitTests)
    fips_files(
        MeshBuilderTest.cc
        ShapeBuilderTest.cc
        VertexWriterTest.cc
    )
    fips_deps(Gfx Assets)
fips_end_unittest()


