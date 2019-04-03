package com.__UT_APP_PKG_NAME__;

import android.app.Activity;
import android.text.method.ScrollingMovementMethod;
import android.text.Layout;
import android.os.Bundle;
import android.os.Handler;

import android.widget.TextView;
import android.widget.EditText;
import android.widget.LinearLayout;

import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.ViewGroup;

import android.view.View.OnKeyListener;
import android.view.View;

import java.util.concurrent.TimeUnit;
import java.util.Timer;
import java.util.TimerTask;

public class __UT_APP_PKG_NAME__ extends Activity
{
// - - - - - - - - - - - - - - - - - - //
/*\
 *  M e m b e r s
\*/
	// text views
	TextView tv;
	EditText et;
	LinearLayout layout;

	// console text buffer
	String output_buffer = "";

	// console text buffer capacity
	int output_capacity = 4096;

	// exit timer
	final Handler exit_handler = new Handler();
    final Timer exit_timer = new Timer();

	// console output timer
	final Handler tv_handler = new Handler();
    final Timer tv_timer = new Timer();

// - - - - - - - - - - - - - - - - - - //
/*\
 *  N a t i v e    l i b r a r y
\*/

	// import library functions
	public native void RunNativeCode();
	public native boolean NativeCodeCompleted();
	public native String FlushOutput();
	public native void ConsoleInput(String cmd);

// - - - - - - - - - - - - - - - - - - //
/*\
 *  A c t i v i t y
\*/

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		// create activity
        super.onCreate(savedInstanceState);

        // create a TextView
        tv = new TextView(this);

		// enable multiple line view
		tv.setSingleLine(false);

		// enable scrolling
		tv.setMovementMethod(new ScrollingMovementMethod());

		// create edit text control
		et = new EditText(this);
		et.setSingleLine(true);

		// layout
		layout = new LinearLayout(this);
        LinearLayout.LayoutParams parms = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setLayoutParams(parms);
		setContentView(layout);
 
        // add text and edittext views
		layout.addView(et, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        layout.addView(tv, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT));

		// start native code
		RunNativeCode();

		// launch exit update task
        TimerTask exit_task = new TimerTask()
		{
            public void run()
            {
                exitUpdate();
            }
        };
		exit_timer.schedule(exit_task,0,200);

		// launch text view update task
        TimerTask tv_task = new TimerTask()
		{
            public void run()
            {
                tvUpdate();
            }
        };
		tv_timer.schedule(tv_task,0,200);

		// text edit 'enter' callback
		TextView.OnEditorActionListener et_listener = new TextView.OnEditorActionListener()
		{
			public boolean onEditorAction(TextView in_tv, int action_id, KeyEvent event)
			{
				if(action_id == EditorInfo.IME_ACTION_DONE)
				{
					// get edit text
					String input = et.getText().toString();

					// hide keyboard and clear edit control
					hideEditKeyboard();
					et.setText("");

					// execute cmd
					ConsoleInput(input);
				}
				return true;
			}
		};

		// assign enter callback
		et.setOnEditorActionListener(et_listener);
    }

	// finish activity when press back button
	@Override
	public void onBackPressed()
	{
		finish();
		android.os.Process.killProcess(android.os.Process.myPid());
	}

	// runnable of exit update task
    final Runnable exitRunnable = new Runnable()
	{
        public void run()
        {
            boolean status = NativeCodeCompleted();
			if(status)
			{
				finish();
				android.os.Process.killProcess(android.os.Process.myPid());
			}
        }
    };

	// runnable of tv update task
    final Runnable tvRunnable = new Runnable()
	{
        public void run()
        {
            // get new output text
			String str = FlushOutput();

			// update output buffer and text view
			tvAppend(str);
        }
    };

	// updates text view
    private void tvAppend(String str)
	{
        if(str!= null && str.isEmpty() == false)
		{
			// append string to the text buffer
			output_buffer += str;

			// crop buffer length
			if(output_buffer.length() > output_capacity)
			{
				String cropped_buffer = output_buffer.substring(output_buffer.length() - output_capacity);
				output_buffer = cropped_buffer;
			}

			// assign new text
			tv.setText(output_buffer);

			// scroll to the bottom
			final Layout tv_layout = tv.getLayout();
			if(tv_layout != null)
			{
				int scrollDelta = tv_layout.getLineBottom(tv.getLineCount() - 1) - tv.getScrollY() - tv.getHeight();
				if(scrollDelta > 0)
				{
					tv.scrollBy(0, scrollDelta);
				}
			}
		}
    }

	// checks if native code completed, and closes the application if so
    private void exitUpdate()
	{
        exit_handler.post(exitRunnable); // relate this to a Runnable
    }

	// updates text view
    private void tvUpdate()
	{
        // num.setText(String.valueOf(i)); = avoid the RunTime error
        tv_handler.post(tvRunnable); // relate this to a Runnable
    }

	private void hideEditKeyboard()
	{
		View focused = getCurrentFocus();
		InputMethodManager imm = (InputMethodManager)getSystemService(Activity.INPUT_METHOD_SERVICE);
		imm.hideSoftInputFromWindow(focused.getWindowToken(), 0);
	}
    
    // load library with native code
	static
	{
		System.loadLibrary("__UT_APP_NAME__");
	}
}
