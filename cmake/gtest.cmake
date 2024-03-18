include_guard()

CPMAddPackage(
	NAME googletest
	GITHUB_REPOSITORY google/googletest
	GIT_TAG v1.14.0
	VERSION 1.14.0
	OPTIONS
	"INSTALL_GTEST OFF"
	"gtest_force_shared_crt ON"
)
set_property(TARGET 
	gtest 
	gtest_main 
	gmock
	gmock_main
	PROPERTY FOLDER third_party/GoogleTest)