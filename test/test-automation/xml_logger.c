/*
  Copyright (C) 2011 Markus Kauppila <markus.kauppila@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL/SDL.h>

#include "xml.h"
#include "logger_helpers.h"

#include "xml_logger.h"

/*! Static strings for XML elements */
const char *documentRoot = "testlog";
const char *parametersElementName = "parameters";
const char *parameterElementName = "parameter";
const char *startTimeElementName = "startTime";
const char *numSuitesElementName = "numSuites";
const char *numTestElementName = "numTests";
const char *numPassedTestsElementName = "numPassedTests";
const char *numFailedTestsElementName = "numFailedTests";
const char *endTimeElementName = "endTime";
const char *totalRuntimeElementName = "totalRuntime";
const char *suiteElementName = "suite";
const char *testsPassedElementName = "testsPassed";
const char *testsFailedElementName = "testsFailed";
const char *testsSkippedElementName = "testsSkipped";
const char *testElementName = "test";
const char *nameElementName = "name";
const char *descriptionElementName = "description";
const char *resultElementName = "result";
const char *assertElementName = "assert";
const char *messageElementName = "message";
const char *timeElementName = "time";
const char *assertSummaryElementName = "assertSummary";
const char *assertCountElementName = "assertCount";
const char *assertsPassedElementName = "assertsPassed";
const char *assertsFailedElementName = "assertsFailed";
const char *logElementName = "log";


/*! Current indentationt level */
static int indentLevel;

//! Constants for XMLOuputters EOL parameter
#define YES 1
#define NO 0

/*! Controls printing the indentation in relation to line breaks */
static int prevEOL = YES;

/*
 * Prints out the given xml element etc.
 *
 * \todo Make the destination of the output changeable (defaults to stdout)
 *
 * \param  identLevel the indent level of the message
 * \param EOL will it print end of line character or not
 * \param the XML element itself
 *
 */
void
XMLOutputter(const int currentIdentLevel,
			 int EOL, const char *message)
{
	if(ValidateString(message)) {
		int ident = 0;
		for( ; ident < currentIdentLevel && prevEOL; ++ident) {
			printf("  "); // \todo make configurable?
		}

		prevEOL = EOL;

		if(EOL) {
			fprintf(stdout, "%s\n", message);
		} else {
			fprintf(stdout, "%s", message);
		}

		fflush(stdout);
	} else {
		fprintf(stdout, "Error: Tried to output invalid string!");
	}

	SDL_free((char *)message);
}

void
XMLRunStarted(int parameterCount, char *runnerParameters[], time_t eventTime,
			 void *data)
{
	char *xslStylesheet = (char *)data;

	char *output = XMLOpenDocument(documentRoot, xslStylesheet);
	XMLOutputter(indentLevel++, YES, output);

	output = XMLOpenElement(parametersElementName);
	XMLOutputter(indentLevel++, YES, output);

	int counter = 0;
	for(counter = 0; counter < parameterCount; counter++) {
		char *parameter = runnerParameters[counter];

		output = XMLOpenElement(parameterElementName);
		XMLOutputter(indentLevel++, NO, output);

		output = XMLAddContent(parameter);
		XMLOutputter(indentLevel, NO, output);

		output = XMLCloseElement(parameterElementName);
		XMLOutputter(--indentLevel, YES, output);
	}

	output = XMLCloseElement(parametersElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLOpenElement(startTimeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(TimestampToString(eventTime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(startTimeElementName);
	XMLOutputter(--indentLevel, YES, output);
}

void
XMLRunEnded(int testCount, int suiteCount, int testPassCount, int testFailCount,
            time_t endTime, double totalRuntime)
{
	// log suite count
	char *output = XMLOpenElement(numSuitesElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(suiteCount));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(numSuitesElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log test count
	output = XMLOpenElement(numTestElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(testCount));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(numTestElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log passed test count
	output = XMLOpenElement(numPassedTestsElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(testPassCount));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(numPassedTestsElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log failed test count
	output = XMLOpenElement(numFailedTestsElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(testFailCount));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(numFailedTestsElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log end timte
	output = XMLOpenElement(endTimeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(TimestampToString(endTime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(endTimeElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log total runtime
	output = XMLOpenElement(totalRuntimeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(DoubleToString(totalRuntime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(totalRuntimeElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLCloseDocument(documentRoot);
	XMLOutputter(--indentLevel, YES, output);
}

void
XMLSuiteStarted(const char *suiteName, time_t eventTime)
{
	char *output = XMLOpenElement(suiteElementName);
	XMLOutputter(indentLevel++, YES, output);

	output = XMLOpenElement(nameElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(suiteName);
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(nameElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLOpenElement(startTimeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(TimestampToString(eventTime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(startTimeElementName);
	XMLOutputter(--indentLevel, YES, output);
}

void
XMLSuiteEnded(int testsPassed, int testsFailed, int testsSkipped,
           time_t endTime, double totalRuntime)
{
	// log tests passed
	char *output = XMLOpenElement(testsPassedElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(testsPassed));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(testsPassedElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log tests failed
	output = XMLOpenElement(testsFailedElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(testsFailed));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(testsFailedElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log tests skipped
	output = XMLOpenElement(testsSkippedElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(testsSkipped));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(testsSkippedElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log tests skipped
	output = XMLOpenElement(endTimeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(TimestampToString(endTime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(endTimeElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log total runtime
	output = XMLOpenElement(totalRuntimeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(DoubleToString(totalRuntime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(totalRuntimeElementName);
	XMLOutputter(--indentLevel, YES, output);


	output = XMLCloseElement(suiteElementName);
	XMLOutputter(--indentLevel, YES, output);
}

void
XMLTestStarted(const char *testName, const char *suiteName,
			  const char *testDescription, time_t startTime)
{
	char * output = XMLOpenElement(testElementName);
	XMLOutputter(indentLevel++, YES, output);

	//Attribute attribute = {"test", "value"};
	//XMLOpenElementWithAttribute("name", &attribute);
	output = XMLOpenElement(nameElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(testName);
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(nameElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLOpenElement(descriptionElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(testDescription);
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(descriptionElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLOpenElement(startTimeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(TimestampToString(startTime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(startTimeElementName);
	XMLOutputter(--indentLevel, YES, output);
}

void
XMLTestEnded(const char *testName, const char *suiteName,
          int testResult, time_t endTime, double totalRuntime)
{
	char *output = XMLOpenElement(resultElementName);
	XMLOutputter(indentLevel++, NO, output);

	if(testResult) {
		if(testResult == 2) {
			output = XMLAddContent("failed. No assert");
		} else {
			output = XMLAddContent("failed");
		}
		XMLOutputter(indentLevel, NO, output);
	} else {
		output = XMLAddContent("passed");
		XMLOutputter(indentLevel, NO, output);
	}

	output = XMLCloseElement(resultElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log total runtime
	output = XMLOpenElement(endTimeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(TimestampToString(endTime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(endTimeElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log total runtime
	output = XMLOpenElement(totalRuntimeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(DoubleToString(totalRuntime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(totalRuntimeElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLCloseElement(testElementName);
	XMLOutputter(--indentLevel, YES, output);
}

void
XMLAssert(const char *assertName, int assertResult, const char *assertMessage,
		  time_t eventTime)
{
	char *output = XMLOpenElement(assertElementName);
	XMLOutputter(indentLevel++, YES, output);

	// log assert name
	output = XMLOpenElement(nameElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(assertName);
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(nameElementName);
	XMLOutputter(--indentLevel, YES, output);


	// log assert result
	output = XMLOpenElement(resultElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent((assertResult) ? "pass" : "failure");
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(resultElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log assert message
	output = XMLOpenElement(messageElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(assertMessage);
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(messageElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log event time
	output = XMLOpenElement(timeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(TimestampToString(eventTime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(timeElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLCloseElement(assertElementName);
	XMLOutputter(--indentLevel, YES, output);
}

void
XMLAssertWithValues(const char *assertName, int assertResult, const char *assertMessage,
		int actualValue, int excpected, time_t eventTime)
{
	char *output = XMLOpenElement(assertElementName);
	XMLOutputter(indentLevel++, YES, output);

	// log assert name
	output = XMLOpenElement(nameElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(assertName);
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(nameElementName);
	XMLOutputter(--indentLevel, YES, output);


	// log assert result
	output = XMLOpenElement(resultElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent((assertResult) ? "pass" : "failure");
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(resultElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log assert message
	output = XMLOpenElement(messageElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(assertMessage);
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(messageElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log event time
	output = XMLOpenElement(timeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(TimestampToString(eventTime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(timeElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLCloseElement(assertElementName);
	XMLOutputter(--indentLevel, YES, output);
}

void
XMLAssertSummary(int numAsserts, int numAssertsFailed,
				 int numAssertsPass, time_t eventTime)
{
	char *output = XMLOpenElement(assertSummaryElementName);
	XMLOutputter(indentLevel++, YES, output);

	output = XMLOpenElement(assertCountElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(numAsserts));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(assertCountElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLOpenElement(assertsPassedElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(numAssertsPass));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(assertsPassedElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLOpenElement(assertsFailedElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(IntToString(numAsserts));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(assertsFailedElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLCloseElement(assertSummaryElementName);
	XMLOutputter(--indentLevel, YES, output);
}

void
XMLLog(const char *logMessage, time_t eventTime)
{
	char *output = XMLOpenElement(logElementName);
	XMLOutputter(indentLevel++, NO, output);

	// log message
	output = XMLOpenElement(messageElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(logMessage);
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(messageElementName);
	XMLOutputter(--indentLevel, YES, output);

	// log eventTime
	output = XMLOpenElement(timeElementName);
	XMLOutputter(indentLevel++, NO, output);

	output = XMLAddContent(TimestampToString(eventTime));
	XMLOutputter(indentLevel, NO, output);

	output = XMLCloseElement(timeElementName);
	XMLOutputter(--indentLevel, YES, output);

	output = XMLCloseElement(logElementName);
	XMLOutputter(--indentLevel, YES, output);
}
