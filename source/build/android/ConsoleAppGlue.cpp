//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "UT.h"
//----------------------------------------------------------------------------//
// Android application starts from java code, thus function 'main' isn't the
// entry point. It will be called by java code using RunNativeCode() function.
// This is a declaration of main() function.
int main();

// Android java code should know, when the native code finished the work.
// Function MainThreadFinished() will be consistently called from java to
// check if it should terminate the application. This 'exit' status is
// contained in a global variable 'g_main_thread_finished'.
ut::Sync<bool> g_main_thread_finished(false);

// This is a job for main() function, it must be run in a separate thread,
// otherwise it will cause a block in java code.
class MainJob : public ut::Job
{
public:
	void Execute()
	{
		main();
		g_main_thread_finished.Set(true);
	}
};

// This is a separate thread for a main() function
ut::ThreadPtr g_main_thread;

//----------------------------------------------------------------------------//
// Exported native functions
extern "C"
{
	JNIEXPORT void JNICALL Java_com___UT_APP_PKG_NAME_____UT_APP_PKG_NAME___RunNativeCode()
	{
		// start main in separate thread
		ut::JobPtr main_job(new MainJob);
		g_main_thread = new ut::Thread(main_job);
	}

	JNIEXPORT jboolean JNICALL Java_com___UT_APP_PKG_NAME_____UT_APP_PKG_NAME___NativeCodeCompleted()
	{
		// check if main thread has finished
		bool status = g_main_thread_finished.Get();

		// jboolean is a uint8 type, convert 'bool' to integer
		return status ? 1 : 0;
	}

	JNIEXPORT jstring JNICALL Java_com___UT_APP_PKG_NAME_____UT_APP_PKG_NAME___FlushOutput(JNIEnv * env, jobject _this)
	{
		// get pending output string
		ut::String output = ut::console.FlushOutput();

		// only 0 - 127 characters allowed
		size_t len = output.Length();
		for(size_t i = 0; i < len; i++)
		{
			if(output[i] >= 127)
			{
				output[i] = '*';
			}
		}

		// return java string
		return env->NewStringUTF(output.GetAddress());
	}

	JNIEXPORT void JNICALL Java_com___UT_APP_PKG_NAME_____UT_APP_PKG_NAME___ConsoleInput(JNIEnv * env, jobject _this, jstring input)
	{
		// get cpp char string
		const char *native_str = env->GetStringUTFChars(input, 0);

		// set console input by hands
		ut::console.Input(native_str);
	}
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//