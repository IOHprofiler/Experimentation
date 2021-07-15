#include <gtest/gtest.h>

#include <clutchlog/clutchlog.h>

int main(int argc, char** argv) {	
	::testing::InitGoogleTest(&argc, argv);

    // Allows to adjust the debugging log parameters for all tests.
    // Binaries will expect the log level, then the depth level, then the file scope regexp.
    auto& dbglog = clutchlog::logger();
    if(argc > 1) {
        // After GoogleTest has removed its arguments.
        dbglog.threshold( dbglog.level_of(argv[1]) );
        if(argc > 2) {
            // TODO we could also handle function and/or line regexp.
            // Maybe it's useless for tests, so it's not handled for the moment.
            dbglog.file( argv[2] );
            if(argc > 3) {
                dbglog.depth( std::atoi(argv[3]) );
                if(argc > 4) {
                    std::cerr << "WARNING: Tests do not handle more than 3 arguments, but you passed " << argc-1 << " arguments." << std::endl;
                    std::cerr << "Usage: <test> [--gtest* options] [level [scope [depth]]" << std::endl;
                    std::cerr << "\tLevel is a string indicating the log threshold, levels are: "
                              << "Critical > Error > Warning > Progress > Note > Info > Debug > XDebug (default: XDebug)." << std::endl;
                    std::cerr << "\tScope is an ECMAscript regexp matching the source files (default: '.*')." << std::endl;
                    std::cerr << "\tDepth is an integer indicating the call stack threshold (default: INTMAX)." << std::endl;
                }
            } // Clutchlog's default already on INTMAX.
        } // Clutchlog's default already on any file.
    } else {
        dbglog.threshold( clutchlog::level::xdebug );
    }
     
	return  RUN_ALL_TESTS();
}
