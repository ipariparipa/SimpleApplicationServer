
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/XmlOutputterHook.h>
#include <cppunit/TextOutputter.h>

#include "datetime_test.h"

#include <memory>
#include <assert.h>
#include <string.h>


#include <cppunit/XmlOutputterHook.h>
#include <cppunit/tools/XmlDocument.h>
#include <cppunit/tools/XmlElement.h>
#include <cppunit/tools/StringTools.h>

#include <sasBasics/logging.h>
#include <sasBasics/streamerrorcollector.h>

#include <iostream>

int main(int argc, char ** argv)
{
    SAS::StreamErrorCollector<std::ostream> ec(std::cerr);
    SAS::Logging::init(argc, argv, ec);

	std::unique_ptr<std::ostream> _outputter_stream_obj;
	std::ostream * outputter_stream = &std::cout;
	
	enum class OutputterType
	{
		Compiler,
		Text,
		XML
	} outputterType = OutputterType::Compiler;
	enum class ParseStatus
	{
		None,
		OutFileName
	} status = ParseStatus::None;
	for (int i = 1; i < argc; ++i)
	{
		assert(argv[i]);
		switch (status)
		{
		case ParseStatus::None:
			if (strcmp(argv[i], "-c") == 0)
				outputter_stream = &std::cout;
			else if (strcmp(argv[i], "-e") == 0)
				outputter_stream = &std::cerr;
			else if (strcmp(argv[i], "-file") == 0)
				status = ParseStatus::OutFileName;
			else if (strcmp(argv[i], "-text") == 0)
				outputterType = OutputterType::Text;
			else if (strcmp(argv[i], "-xml") == 0)
				outputterType = OutputterType::XML;
			else if (strcmp(argv[i], "-compiler") == 0)
				outputterType = OutputterType::Compiler;
			else
			{
//				std::cerr << "invalid command line option: '" << argv[i] << "'" << std::endl;
//				exit(1);
			}
			break;
		case ParseStatus::OutFileName:
			_outputter_stream_obj.reset(outputter_stream = new std::ofstream(argv[i]));
			status = ParseStatus::None;
			break;
		}
	}

	// Create the event manager and test controller
	CPPUNIT_NS::TestResult controller;

	// Add a listener that colllects test result
	CPPUNIT_NS::TestResultCollector result;
	controller.addListener(&result);

	// Add a listener that print dots as test run.
	CPPUNIT_NS::BriefTestProgressListener progress;
	controller.addListener(&progress);

    CPPUNIT_NS::TestRunner runner;
    runner.addTest(CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest());
    do
    {
        runner.run(controller);
    } while(false);

	// Print test in a compiler compatible format.

	std::unique_ptr<CPPUNIT_NS::Outputter> outputter;

	switch (outputterType)
	{
	case OutputterType::Compiler:
		outputter.reset(new CPPUNIT_NS::CompilerOutputter(&result, *outputter_stream));
		break;
	case OutputterType::XML:
		{
			auto xml_out = new CPPUNIT_NS::XmlOutputter(&result, *outputter_stream);
			outputter.reset(xml_out);
		}
		break;
	case OutputterType::Text:
		outputter.reset(new CPPUNIT_NS::TextOutputter(&result, *outputter_stream));
		break;
	}

	assert(outputter);
	outputter->write();

	return result.wasSuccessful() ? 0 : 1;
}
