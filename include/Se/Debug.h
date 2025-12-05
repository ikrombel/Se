////********************************** bs::framework - Copyright 2018 Marko Pintera **************************************//
////********* Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

//#include "Prerequisites/BsPrerequisitesUtil.h"
//#include "Debug/BsLog.h"

#include <functional>
#include <Se/String.hpp>
#include <Se/Signal.hpp>
#include <Se/Console.hpp>
//#include <Se/
#include <stdint.h>

namespace Se
{
	class Log;

	/// @addtogroup Debug
	///  @{
	
	/// Type of the log that will be saved.
	enum class SavedLogType
	{
		HTML = 0,
		Textual = 1
	};

	struct DebugEntry {
		Console::ConsoleInfo info_;
		Se::String msg_;
		int time_;
	};
	
	/// Utility class providing various debug functionality.
	///
	/// @note	Thread safe.
	class Debug
	{
		std::vector<DebugEntry> entries_;

	public:
		Debug() = default;

		virtual ~Debug();

		/// Logs a new message.
		///
		/// @param[in]	message		The message describing the log entry.
		/// @param[in]	verbosity	Verbosity of the message, determining its importance.
		/// @param[in]	category	Category of the message, determining which system is it relevant to.
		void log(const String& message, Console::MsgType verbosity, uint32_t category = 0);

		/// Retrieves the Log used by the Debug instance.
		//Log& getLog() { return mLog; }

		/// Converts raw pixels into a BMP image and saves it as a file */
		void writeAsBMP(uint8_t* rawPixels, uint32_t bytesPerPixel, uint32_t width, uint32_t height, const Se::String& filePath,
			bool overwrite = true) const;

		/// Saves a log about the current state of the application to the specified location.
		///
		/// @param	path	Absolute path to the log filename.
		/// @param   type    Format of the saved log.
		void saveLog(const Se::String& path, SavedLogType type = SavedLogType::HTML) const;
		
		/// Saves a log about the current state of the application to the specified location as a HTML file.
		///
		/// @param	path	Absolute path to the log filename.
		void saveHtmlLog(const Se::String& path) const;
		
		/// Saves a log about the current state of the application to the specified location as a text file.
		///
		/// @param	path	Absolute path to the log filename.
		void saveTextLog(const Se::String& path) const;
		
		///
		/// Triggered when a new entry in the log is added.
		/// 			
		/// @note	Sim thread only.
		Signal<const Console::ConsoleInfo&> onLogEntryAdded;
		Console::LogCallback onLog();

		/// Triggered whenever one or multiple log entries were added or removed. Triggers only once per frame.
		/// 			
		/// @note	Sim thread only.
		Signal<> onLogModified;

		/// This allows setting a log callback that can override the default action in log */
		void setLogCallback(
			std::function<bool(const String& message, Console::MsgType verbosity, uint32_t category)> callback)
		{
			mCustomLogCallback = callback;
		}

	public: // ***** INTERNAL ******
		/// @name Internal
		///  @{

		/// Triggers callbacks that notify external code that a log entry was added.
		/// 			
		/// @note	Sim thread only.
		void _triggerCallbacks();

		/// @}
	private:
		uint64_t mLogHash = 0;
		//Log mLog;
		std::function<bool(const String& message, Console::MsgType verbosity, uint32_t category)> mCustomLogCallback;
	};

	/// A simpler way of accessing the Debug module.
	Debug& gDebug();

#ifndef SE_LOG_VERBOSITY
	#if SE_DEBUG_MODE
		#define SE_LOG_VERBOSITY Console::MsgType::Log
	#else
		#define SE_LOG_VERBOSITY Console::MsgType::Warning
	#endif
#endif
}
