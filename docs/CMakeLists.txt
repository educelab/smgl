# Set Doxygen options
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_LATEX YES)
set(DOXYGEN_STRIP_FROM_INC_PATH ${PROJECT_SOURCE_DIR}/smgl/include/)
set(DOXYGEN_STRIP_FROM_PATH ${PROJECT_SOURCE_DIR}/smgl/include/)
set(DOXYGEN_PROJECT_BRIEF "Structured Metadata Engine and Graph Objects Library")
set(DOXYGEN_PROJECT_LOGO "${PROJECT_SOURCE_DIR}/docs/logo.png")
set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")
set(DOXYGEN_SOURCE_BROWSER YES)
set(DOXYGEN_EXTRACT_PRIVATE YES)
set(DOXYGEN_PREDEFINED "SMGL_USE_STDFS=${SMGL_USE_STDFS}")
set(DOXYGEN_IMAGE_PATH "${PROJECT_SOURCE_DIR}/graphics/")
set(DOXYGEN_QUIET YES)

doxygen_add_docs(
    docs
    ${PROJECT_SOURCE_DIR}/README.md
    ${PROJECT_SOURCE_DIR}/docs/Modules.dox
    ${PROJECT_SOURCE_DIR}/smgl/include/
    COMMENT "Generate Doxygen documentation"
)
