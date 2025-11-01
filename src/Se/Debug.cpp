#include "Debug.h"
// #include "Debug/BsLog.h"
#include <Se/Exception.hpp>
//#include "BsBitmapWriter.h"


#include <Se/IO/File.h>
#ifndef ANDROID
#include <Se/Utils/SystemInfo.hpp>
#endif

namespace Se
{
Debug::~Debug() {
	saveLog("./SE_LOG.html", SavedLogType::HTML);
}

// SE_LOG_CATEGORY_IMPL(Uncategorized)
// SE_LOG_CATEGORY_IMPL(FileSystem)
// SE_LOG_CATEGORY_IMPL(RTTI)
// SE_LOG_CATEGORY_IMPL(Generic)
// SE_LOG_CATEGORY_IMPL(Platform)

Console::LogCallback Debug::onLog() {
	return [this](const Console::ConsoleInfo& info, const char *msg) {
		DebugEntry entry;
		entry.info_ = info;
		entry.msg_ = Se::String(msg);
		entry.time_ = 0;
		entries_.push_back(entry);
	};
}

void Debug::saveLog(const Se::String& path, SavedLogType type) const
{
	switch (type)
	{
	default:
	case SavedLogType::HTML:
		saveHtmlLog(path);
		break;
	case SavedLogType::Textual:
		saveTextLog(path);
		break;
	}
}
	
	void Debug::saveHtmlLog(const String& path) const
	{
		static const char* style = R"(
html {
  font-family: sans-serif;
  background-color: #1e1720;
  color: #bbbbbb
}
			
table
{
	border-collapse: collapse;
	border-spacing: 0;
	empty-cells: show;
	border: 2px solid #4c3a52;
	width:100%;
	table-layout:fixed;
}

table caption
{
	color: #000;
	font: italic 85%/1 arial, sans-serif;
	padding: 1em 0;
	text-align: center;
}

table td,
table th
{
	background-color: #272131;
	/*border-left: 2px solid #cbcbcb;/*  inner column border */
	border-width: 0 0 0 1px;
	font-size: inherit;
	margin: 0;
	overflow: visible; /*to make ths where the title is really long work*/
	padding: 0.5em 1em; /* cell padding */
}

table td:first-child,
table th:first-child
{
	border-left-width: 0;
}

table thead
{
	background-color: #272131;
	color: #AAA;
	text-align: left;
	vertical-align: bottom;
}

table td
{
	background-color: transparent;
	word-wrap: break-word;
	vertical-align: top;
	color: #d3d3d3;
}

.debug-row td {
	background-color: #414141;
}

.debug-alt-row td {
	background-color: #252525;
}

.warn-row td {
	background-color: #ffc016;
	color: #353535;
}

.warn-alt-row td {
	background-color: #fdcb41;
	color: #353535;
}

.error-row td {
	background-color: #9f1621;
	color: #dfdfdf;
}

.error-alt-row td {
	background-color: #ae1621;
	color: #dfdfdf;
}

)";

		static const char* htmlPreStyleHeader = R"(
<!DOCTYPE html>
<html lang="en">
<head>
<style type="text/css">
)";

		static const char* htmlPostStyleHeader = R"(
</style>
<title>Se Log</title>
</head>
<body>
)";	

		static const char* htmlEntriesTableHeader =	R"(
<table border="1" cellpadding="1" cellspacing="1">
	<thead>
		<tr>
			<th scope="col" style="width:85px">Type</th>
			<th scope="col" style="width:70px">Time</th>
			<th scope="col" style="width:160px">Category</th>
			<th scope="col">Description</th>
		</tr>
	</thead>
	<tbody>
)";

		static const char* htmlFooter =	R"(
	</tbody>
</table>
</body>
</html>)";

		std::stringstream stream;
		stream << htmlPreStyleHeader;
		stream << style;
		stream << htmlPostStyleHeader;
		stream << "<h1>Se Log</h1>\n";
		stream << "<h2>System information</h2>\n";

		// Write header information
		stream << "<p>Se::framework version: 1.0.0</p>\n";

		// if(Time::isStarted())
		// 	stream << "<p>Started on: " << gTime().getAppStartUpDateString(false) << "</p>\n";
#ifndef ANDROID
		SystemInfo systemInfo = Platform::getSystemInfo();
		stream << format("<p>OS version: {} {}</p>\n", systemInfo.osName, systemInfo.osIs64Bit ? "64-bit" : "32-bit");
		stream << "<h3>CPU information:</h3>\n";
		stream << format(
			"<p>CPU vendor: {} </p>\n"
			"<p>CPU name: {}</p>\n"
			"<p>CPU clock speed: {}Mhz</p>\n"
			"<p>CPU core count: {}</p>\n", 
			systemInfo.cpuManufacturer, systemInfo.cpuModel, 
			systemInfo.cpuClockSpeedMhz, systemInfo.cpuNumCores);
#endif
		// stream << "<h3>GPU List:</h3>\n";
		// if (systemInfo.gpuInfo.numGPUs == 1)
		//  	stream << format("<p>GPU: {}</p>\n", systemInfo.gpuInfo.names[0]);
		// else
		// {
		// 	for(uint32_t i = 0; i < systemInfo.gpuInfo.numGPUs; i++)
		//  		stream << "<p>GPU #" << i << ": " << systemInfo.gpuInfo.names[i] << "</p>\n";
		// }

		// Write log entries
		stream << "<h2>Log entries</h2>\n";
		stream << htmlEntriesTableHeader;

		bool alternate = false;
		//std::vector<Console::ConsoleInfo> entries; // = mLog.getAllEntries();
		for (auto& entry : entries_)
		{
			auto type0 = entry.info_.type_;

			if (type0 == Console::MsgType::MsgInfo  ||
				type0 == Console::MsgType::MsgDebug ||
				type0 == Console::MsgType::MsgNone)
				continue;

			String channelName;
			String alternateName = alternate ? "-alt-row" : "-row";

			Console::MsgType verbosity = entry.info_.type_;
			switch(verbosity)
			{
			//case Console::MsgType::Fatal:
			case Console::MsgType::MsgError:
				stream << format(R"(		<tr class="error{}">)", alternateName) << std::endl;
				break;
			case Console::MsgType::MsgWarning: {
				// std::string alt =  (!alternate) ? "warn-row" : "warn-alt-row";
				// stream << format(R"(		<tr class=\"{}\">)", alt);
				stream << format(R"(		<tr class="warn{}">)", alternateName) << std::endl;
			}
			default:
			case Console::MsgType::MsgInfo:
			// case Console::MsgType::Log:
			// case Console::MsgType::Verbose:
			// case Console::MsgType::VeryVerbose:
					stream << format(R"(		<tr class="debug{}">)", alternateName) << std::endl;
				break;
			}
			// stream << R"(			<td>)" << toString(verbosity)<< R"(</td>)" << std::endl;
			// stream << R"(			<td>)" << toString(entry.getLocalTime(), false, false, TimeToStringConversionType::Time) << "</td>" << std::endl;

			stream << R"(			<td>TODO:verbosity</td>)" << std::endl;
			stream << R"(			<td>TODO:Time</td>)" << std::endl;
			

			String categoryName = entry.info_.name_;
			stream << R"(			<td>)" << categoryName << "</td>" << std::endl;

			String parsedMessage =  entry.msg_.replaced("\n", "<br>\n");
			stream << R"(			<td>)" << parsedMessage << "</td>" << std::endl;
			stream << R"(		</tr>)" << std::endl;

			alternate = !alternate;
		}

		stream << htmlFooter;

		File file(path, FileMode::FILE_WRITE);
		file.WriteText(stream.str());

		// SPtr<DataStream> fileStream = FileSystem::createAndOpenFile(path);
		// fileStream->writeString(stream.str());
	}

	void Debug::saveTextLog(const Se::String& path) const
	{

#if 0
		#if BS_IS_BANSHEE3D
		static const char* engineHeader = "This is Se Engine ";
		static const char* bsfBasedHeader = "Based on Se::framework ";
		#else
		static const char* bsfOnlyHeader = "This is Se ";
		#endif
		
		std::stringstream stream;
		#if BS_IS_BANSHEE3D
		stream << engineHeader << BS_B3D_VERSION_MAJOR << "." << BS_B3D_VERSION_MINOR << "." << BS_B3D_VERSION_PATCH << "\n";
		stream << bsfBasedHeader << BS_VERSION_MAJOR << "." << BS_VERSION_MINOR <<"." << BS_VERSION_PATCH << "\n";
		#else
		stream << bsfOnlyHeader << BS_VERSION_MAJOR << "." << BS_VERSION_MINOR <<"." << BS_VERSION_PATCH << "\n";
		#endif
		if (Time::isStarted())
			stream << "Started on: " << gTime().getAppStartUpDateString(false) << "\n";
		
		stream << "\n";
		stream << "System information:\n" <<
				  "================================================================================\n";
	
		SystemInfo systemInfo = PlatformUtility::getSystemInfo();
		stream << "OS version: " << systemInfo.osName << " " << (systemInfo.osIs64Bit ? "64-bit" : "32-bit") << "\n";
		stream << "CPU information:\n";
		stream << "CPU vendor: " << systemInfo.cpuManufacturer << "\n";
		stream << "CPU name: " << systemInfo.cpuModel << "\n";
		stream << "CPU clock speed: " << systemInfo.cpuClockSpeedMhz << "Mhz\n";
		stream << "CPU core count: " << systemInfo.cpuNumCores << "\n";
		
		stream << "\n";
		stream << "GPU List:\n" <<
				  "================================================================================\n";
		
		if (systemInfo.gpuInfo.numGPUs == 1)
			stream << "GPU: " << systemInfo.gpuInfo.names[0] << "\n";
		else
		{
			for(uint32_t i = 0; i < systemInfo.gpuInfo.numGPUs; i++)
				stream << "GPU #" << i << ": " << systemInfo.gpuInfo.names[i] << "\n";
		}
		
		stream << "\n";
		stream << "Log entries:\n" <<
				  "================================================================================\n";
		
		std::vector<Console::ConsoleInfo> entries = mLog.getAllEntries();
		for (auto& entry : entries)
		{
			String builtMsg;
			builtMsg.append(toString(entry.getLocalTime(), false, true, TimeToStringConversionType::Full));
			builtMsg.append(" ");
			
			switch(entry.getVerbosity())
			{
			case Console::MsgType::Fatal:
				builtMsg.append("[FATAL]");
				break;
			case Console::MsgType::Error:
				builtMsg.append("[ERROR]");
				break;
			case Console::MsgType::Warning:
				builtMsg.append("[WARNING]");
				break;
			case Console::MsgType::Info:
				builtMsg.append("[INFO]");
				break;
			case Console::MsgType::Log:
				builtMsg.append("[LOG]");
				break;
			case Console::MsgType::Verbose:
				builtMsg.append("[VERBOSE]");
				break;
			case Console::MsgType::VeryVerbose:
				builtMsg.append("[VERY_VERBOSE]");
				break;
			}
			
			String categoryName;
			mLog.getCategoryName(entry.getCategory(), categoryName);
			builtMsg.append(" <" + categoryName + ">");

			builtMsg.append(" | ");
			
			String tmpSpaces = _getSpacesIndentation(builtMsg.length());
			
			String parsedMessage = StringUtil::replaceAll(entry.getMessage(), "\n\t\t", "\n" + tmpSpaces);
			builtMsg.append(parsedMessage);
			
			stream << builtMsg << "\n";
		}
		
		SPtr<DataStream> fileStream = FileSystem::createAndOpenFile(path);
		fileStream->writeString(stream.str());
#endif
	}
	
	Debug& gDebug()
	{
		static Debug debug;
		return debug;
	}
}
