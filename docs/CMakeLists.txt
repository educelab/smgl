# Set Doxygen options
set(DOXYGEN_DOT_IMAGE_FORMAT svg)
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_LATEX YES)
set(DOXYGEN_HTML_EXTRA_STYLESHEET ${PROJECT_SOURCE_DIR}/docs/style/doxygen_extra.css)
set(DOXYGEN_STRIP_FROM_INC_PATH ${PROJECT_SOURCE_DIR}/smgl/include/)
set(DOXYGEN_STRIP_FROM_PATH ${PROJECT_SOURCE_DIR}/smgl/include/)
set(DOXYGEN_PROJECT_BRIEF "Structured Metadata Engine and Graph Objects Library")
set(DOXYGEN_PROJECT_LOGO "${PROJECT_SOURCE_DIR}/docs/logo.png")
set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/docs/index.md")
set(DOXYGEN_SOURCE_BROWSER YES)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_EXTRACT_PRIVATE YES)
set(DOXYGEN_IMAGE_PATH "${PROJECT_SOURCE_DIR}/graphics/")
set(DOXYGEN_QUIET YES)

# Setup tag files
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/doxy-tags/)
set(DOXYGEN_GENERATE_TAGFILE ${CMAKE_CURRENT_BINARY_DIR}/html/doxy-tags/educelab_smgl.tag)
file(COPY doxy-tags/ DESTINATION html/doxy-tags/)
set(DOXYGEN_TAGFILES "doxy-tags/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/")

# Add compiler definitions
if(SMGL_USE_BOOSTFS)
    set(DOXYGEN_PREDEFINED "SMGL_USE_BOOSTFS=${SMGL_USE_BOOSTFS}")
endif()

doxygen_add_docs(
    docs
    ${PROJECT_SOURCE_DIR}/docs/pages/
    ${PROJECT_SOURCE_DIR}/docs/index.md
    ${PROJECT_SOURCE_DIR}/docs/Modules.dox
    ${PROJECT_SOURCE_DIR}/smgl/include/
    COMMENT "Generate Doxygen documentation"
)

