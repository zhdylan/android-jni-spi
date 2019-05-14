package com.picovr.cvcontrol;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;


public class MainActivity extends Activity {
		
	private static final String ACTION_CV_START_THREAD = "com.picovr.picovrlib.cv.broadcast.start.thread";
	private static final String ACTION_CV_STOP_THREAD = "com.picovr.picovrlib.cv.broadcast.stop.thread";
	public native void startStationOTAUpdate();
	public native void startControllerOTAUpdate(int mode ,int whichController);
	public native String getProcess();
	public native String getDevicesVersion(int device);
	public native void setRatio(int num);
	public native void enterPairMode(int whichController);//1 or 2
	public native int isenterPairMode();
	public native String getControllerSN();
	public native void controllervibrate();
	public native void cancelcontrollervibrate();
	public native void setControllerUnbinding(int whichController);
	public native void setControllerShutdown(int whichController);
	
	static {
		System.loadLibrary("ota-jni");
	}
	    
	private Button btn_StartStationUpdate;
	private Button btn_StartControllerUpdate;
	private Button btn_StartStm32Update;
	private Button btn_GetStationVersion;
	private Button btn_GetControllerVersion;
	private Button btn_Controllervibrate;
	private Button btn_CancelControllervibrate;
	private Button btn_EnterPair1;
	private Button btn_EnterPair2;
	private Button btn_isEnterPair;
	private Button btn_unbind;
	private Button btn_shutdown;
	
	private ProgressBar btn_ProgressBar;
	private TextView textprogress;
	private TextView textShow;
	private Handler mHandler = new Handler();  
  
	private int progress = 0;
	private String version = "get version fail";
	private String device = "";
	private int PairStatus = 0;
	private Timer timer = null;
	private boolean pairSucess = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.requestWindowFeature(Window.FEATURE_NO_TITLE); 
        setContentView(R.layout.activity_main);

        btn_StartStationUpdate = (Button) findViewById(R.id.startStationUpdate);
        btn_StartControllerUpdate = (Button) findViewById(R.id.startControllerUpdate);
        btn_StartStm32Update = (Button) findViewById(R.id.startSTM32Update);
        btn_GetStationVersion = (Button) findViewById(R.id.getStationVersion);
        btn_GetControllerVersion = (Button) findViewById(R.id.getControllerVersion);
        btn_Controllervibrate = (Button) findViewById(R.id.vibrate);
        btn_CancelControllervibrate = (Button) findViewById(R.id.cancelvibrate);
        btn_EnterPair1 = (Button) findViewById(R.id.enterPair1);
        btn_EnterPair2 = (Button) findViewById(R.id.enterPair2);
        btn_unbind = (Button) findViewById(R.id.unbindController);
        btn_shutdown = (Button) findViewById(R.id.shutdownController);
        
        btn_isEnterPair= (Button) findViewById(R.id.isenterPair);
        btn_ProgressBar = (ProgressBar) findViewById(R.id.progressBar);
        textprogress = (TextView) findViewById(R.id.progressshow);
        textShow = (TextView) findViewById(R.id.textShow);
                   
        btn_StartStationUpdate.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				btn_StartStationUpdate.setEnabled(false);
				// TODO Auto-generated method stub				
				btn_ProgressBar.setVisibility(View.VISIBLE);
				textprogress.setVisibility(View.VISIBLE);
				
				startStationUpdate();	
				getStationUpdateProgress();
				btn_StartStationUpdate.setEnabled(true);
			}
		});
        
        btn_StartControllerUpdate.setOnClickListener(new OnClickListener() {
 
			@Override
			public void onClick(View v) {
		       	//btn_StartControllerUpdate.setEnabled(false);
				// TODO Auto-generated method stub
				btn_ProgressBar.setVisibility(View.VISIBLE);
				textprogress.setVisibility(View.VISIBLE);
		    	startControllerUpdate(1,1);
				getControllerUpdateProgress("controller");
			

				//btn_StartControllerUpdate.setEnabled(true);
			}
		});
        
        btn_StartStm32Update.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				btn_ProgressBar.setVisibility(View.VISIBLE);
				textprogress.setVisibility(View.VISIBLE);
				startControllerUpdate(2,1);
				getControllerUpdateProgress("stm32");
			}
		});
        btn_GetStationVersion.setOnClickListener(new OnClickListener() {
			
			/* (non-Javadoc)
			 * @see android.view.View.OnClickListener#onClick(android.view.View)
			 */
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				btn_GetStationVersion.setEnabled(false);
				textShow.setVisibility(View.VISIBLE);
				new Thread(new Runnable() {
					@Override
					public void run() {			
				         getVersion(0);//0 station ,1 controller 1,2 controller 2
					}
				}).start();
			}		
		});
        
        btn_GetControllerVersion.setOnClickListener(new OnClickListener() {
			
			/* (non-Javadoc)
			 * @see android.view.View.OnClickListener#onClick(android.view.View)
			 */
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				btn_GetControllerVersion.setEnabled(false);
				textShow.setVisibility(View.VISIBLE);
				new Thread(new Runnable() {
					@Override
					public void run() {			
					
						getVersion(1);//0 station ,1 controller 1,2 controller 2
					}
				}).start();
				
			}		
		});
        
        btn_Controllervibrate.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub				
				new Thread(new Runnable() {
					@Override
					public void run() {			
					
						controllervibrate();
					}
				}).start();

			}
		});
        btn_CancelControllervibrate.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				new Thread(new Runnable() {
					@Override
					public void run() {			
					
						cancelcontrollervibrate();
					}
				}).start();
				
			}
		});
        btn_EnterPair1.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
			
				new Thread(new Runnable() {
					@Override
					public void run() {			
					
						enterPairMode(1);
					}
				}).start();

			}
		});
        
       btn_EnterPair2.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub				
				new Thread(new Runnable() {
					@Override
					public void run() {			
					
						enterPairMode(2);
					}
				}).start();
			}
		});
              
        btn_isEnterPair.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				btn_isEnterPair.setEnabled(false);
				textShow.setVisibility(View.VISIBLE);
				pairSucess = false;
				new Thread(new Runnable() {
					@Override
					public void run() {			
						PairStatus = -1;
						PairStatus = isenterPairMode();
					}
				}).start();
				try {
					Thread.sleep(200);
				} catch (InterruptedException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
				timer = new Timer();  
		        timer.schedule(new getPairStatusTask(), 0,100);  	
		        new Thread(new Runnable() {
					@Override
					public void run() {	
						for(int i=0 ; i< 70 ; i++)
						{
							try {
								Thread.sleep(100);
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
							if(pairSucess)
								break;
						}
						
						if( !pairSucess )
						{
							if(timer!=null)
							{
								timer.cancel();
							}
							MainActivity.this.runOnUiThread(new Runnable() {
								@Override
								public void run() {
									textShow.setText("获取连接状态失败");
									btn_isEnterPair.setEnabled(true);
								}
							});
						}												
					}
				}).start();
			}
		});
        
        btn_unbind.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				new Thread(new Runnable() {
					@Override
					public void run() {			
					
						setControllerUnbinding(1);
					}
				}).start();
				
			}
		});
        btn_shutdown.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				new Thread(new Runnable() {
					@Override
					public void run() {			
					
						setControllerShutdown(1);
					}
				}).start();
				
			}
		});
             
    }
    private void getVersion(final int mode) {
    	version = "";
    	device = "";
    	if( mode == 0 )
    	{
    		device = "station";
    	}else if( mode == 1)
    	{
    		device = "controller";
    	}
		new Thread(new Runnable() {
			@Override
			public void run() {								
				version = getDevicesVersion(mode);	// 0 station version
				                                // 1 controller 1 version,2 controller 2 version
			}
		}).start();
		 
		for(int i=0;i < 30;i++)
		{
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			if(!version.equals(""))
				break;
		}
		
		
		MainActivity.this.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				Log.i("CVSPI",version);
				if(!version.equals("")){
					try {
						String nordic = version.substring(0,29).trim();					
						String stm_32 = version.substring(29,57).trim();
						Log.i("johnson","nordic = "+nordic);
						Log.i("johnson","stm_32 = "+stm_32);
						textShow.setText(nordic +"\n"+stm_32);
					} catch (Exception e) {
						// TODO: handle exception					
						textShow.setText(version +"\n");
					}											
				}else
				{
					textShow.setText("获取"+device+"版本失败");
				}
				if (mode == 0) {
					btn_GetStationVersion.setEnabled(true);
				} else if (mode == 1) {
					btn_GetControllerVersion.setEnabled(true);
				}
			}
		});
		
	}
    private void startControllerUpdate(final int mode,final int whichController) {
		new Thread(new Runnable() {
			@Override
			public void run() {							
			  startControllerOTAUpdate(mode,whichController);					
			}
		}).start();
	}

	private void startStationUpdate() {
		new Thread(new Runnable() {
			@Override
			public void run() {							
			  startStationOTAUpdate();					
			}
		}).start();
	}

	private void getControllerUpdateProgress(final String TAG) {
		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					progress = Integer.parseInt(getProcess());
					//Log.i("johnson ", "progress = " + progress);
					mHandler.post(new Runnable() {
						@Override
						public void run() {
							if (progress == 0) 
							{
								btn_StartStationUpdate.setEnabled(false);
								btn_StartControllerUpdate.setEnabled(false);
								btn_StartStm32Update.setEnabled(false);
								btn_GetStationVersion.setEnabled(false);
								btn_GetControllerVersion.setEnabled(false);
								btn_Controllervibrate.setEnabled(false);
								btn_CancelControllervibrate.setEnabled(false);
								btn_EnterPair1.setEnabled(false);
								btn_EnterPair2.setEnabled(false);
								btn_isEnterPair.setEnabled(false);
								btn_unbind.setEnabled(false);
								btn_shutdown.setEnabled(false);
							}
							
							if (progress <= 100) {
								btn_ProgressBar.setProgress(progress);
								textprogress.setText(TAG+"升级进度:" + progress + "%");
							}
														
						}
					});	

					if (progress == 100 ||progress == 101||progress == 102) {
						setRatio(0);
						break;
					}
					try {
						Thread.sleep(100);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}

				}
				
				MainActivity.this.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						// TODO Auto-generated method stub		
						if (progress == 100)
						{
							textprogress.setText(TAG+"升级完成");
						}else if(progress == 101)
						{
							textprogress.setText(TAG+"升级失败");
						}else if(progress == 102)
						{
							textprogress.setText(TAG+"未准备好，请重启controller");
						}
						btn_StartStationUpdate.setEnabled(true);
						btn_StartControllerUpdate.setEnabled(true);
						btn_StartStm32Update.setEnabled(true);
						btn_GetStationVersion.setEnabled(true);
						btn_GetControllerVersion.setEnabled(true);
						btn_Controllervibrate.setEnabled(true);
						btn_CancelControllervibrate.setEnabled(true);
						btn_EnterPair1.setEnabled(true);
						btn_EnterPair2.setEnabled(true);
						btn_isEnterPair.setEnabled(true);
						btn_unbind.setEnabled(true);
						btn_shutdown.setEnabled(true);
					}
				});				
			}
		}).start();
	}
	private void getStationUpdateProgress() {
		new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					progress = Integer.parseInt(getProcess());
					//Log.i("johnson ", "progress = " + progress);

					mHandler.post(new Runnable() {
						@Override
						public void run() {
							if (progress == 0) 
							{
								btn_StartStationUpdate.setEnabled(false);
								btn_StartControllerUpdate.setEnabled(false);
								btn_StartStm32Update.setEnabled(false);
								btn_GetStationVersion.setEnabled(false);
								btn_GetControllerVersion.setEnabled(false);
								btn_Controllervibrate.setEnabled(false);
								btn_CancelControllervibrate.setEnabled(false);
								btn_EnterPair1.setEnabled(false);
								btn_EnterPair2.setEnabled(false);
								btn_isEnterPair.setEnabled(false);
								btn_unbind.setEnabled(false);
								btn_shutdown.setEnabled(false);
							}
							if (progress == 101) {
								textprogress.setText("升级失败，请重新升级");
							}
							if (progress <= 100) {
								btn_ProgressBar.setProgress(progress);
								textprogress.setText("Station升级进度:" + progress + "%");
							}
							
							if (progress == 100) {
								textprogress.setText("正在对station进行初始化，请勿有任何操作");	

							}
							
						}
					});	
					Log.i("johnson","progress = "+progress);
					if (progress == 100 || (progress == 101)) {
						setRatio(0);
						break;
					}
					try {
						Thread.sleep(100);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}

				}
				if( progress == 100)
				{
					try {
						Thread.sleep(30*1000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					
					Log.i("johnson","station初始化完成");
					//Log.i("johnson","version ="+getStationVersion());
				}
				MainActivity.this.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						// TODO Auto-generated method stub		
						if (progress == 100)
						{
							textprogress.setText("station初始化完成");
						}
						btn_StartStationUpdate.setEnabled(true);
						btn_StartControllerUpdate.setEnabled(true);
						btn_StartStm32Update.setEnabled(true);
						btn_GetStationVersion.setEnabled(true);
						btn_GetControllerVersion.setEnabled(true);
						btn_Controllervibrate.setEnabled(true);
						btn_CancelControllervibrate.setEnabled(true);
						btn_EnterPair1.setEnabled(true);
						btn_EnterPair2.setEnabled(true);
						btn_isEnterPair.setEnabled(true);
						btn_unbind.setEnabled(true);
						btn_shutdown.setEnabled(true);
					}
				});				
			}
		}).start();
	}
	class getPairStatusTask extends TimerTask {
        public void run() {
        	if(PairStatus != -1 )
        	{
        		pairSucess = true;
        		MainActivity.this.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						if (PairStatus == 0) {
							Log.i("CVSPI", " status = 未进入配对状态");
							textShow.setText("未进入配对状态");
						} else if (PairStatus == 1) {
							Log.i("CVSPI", " status = 已进入配对状态");
							textShow.setText("已进入配对状态");
						} else if (PairStatus == 2) {
							Log.i("CVSPI", " status = 已连接controller1");
							textShow.setText("已连接controller1");
						} else if (PairStatus == 3) {
							Log.i("CVSPI", " status = 已连接controller2");
							textShow.setText("已连接controller2");
						} else if (PairStatus == 4) {
							Log.i("CVSPI", " status = 已连接 controller1、controller2");
							textShow.setText("已连接controller1、controller2");
						}
						Log.i("CVSPI", "btn_isEnterPair status = " + PairStatus);	
						if (timer != null)
							timer.cancel();
						btn_isEnterPair.setEnabled(true);
					}
        		});
        	}       	
        }
    }
   private void loadBin()
   {
	   try {
			Log.i("OTA", "start loadBIn ");
			DataInputStream read = new DataInputStream(new FileInputStream(new File("/mnt/sdcard/mybin/6dof.bin")));
			if(read!=null)
			{
				Log.i("OTA", "loadBIn suncess");
			}else
			{
				Log.i("OTA", "loadBIn fail");
			}
							
			byte []b = new byte[24439];										
			if(read.read(b)!=-1)
			{
				Log.i("OTA", "length = "+b.length);
				String result = bytesToHexString(b);
				
				//Log.i("johnson", result);
			}
			
		
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		
	}
   public static String bytesToHexString(byte[] src){  
       StringBuilder stringBuilder = new StringBuilder("");  
       if (src == null || src.length <= 0) {  
           return null;  
       }  
       int line=0;
       for (int i = 0; i < src.length; i++) {  
           int v = src[i] & 0xFF;  
           String hv = Integer.toHexString(v);  
           if (hv.length() < 2) {  
               stringBuilder.append(0);  
           }  
           stringBuilder.append(hv); 
           line++;
           if(line%16==0)
             stringBuilder.append("\n");
           else
           {
           	stringBuilder.append("  ");
           }
       } 
       stringBuilder.append("\n\n line = "+line);
       return stringBuilder.toString();  
   }  
   
	private Handler mHandler_delay = new Handler()
	{
		 @Override
	        public void handleMessage(Message msg) {
	            super.handleMessage(msg);
	            switch (msg.what) {
	                case 0:
	                	btn_StartStationUpdate.setEnabled(true);
						btn_StartControllerUpdate.setEnabled(true);
						btn_StartStm32Update.setEnabled(true);
						btn_GetStationVersion.setEnabled(true);
						btn_GetControllerVersion.setEnabled(true);
						btn_Controllervibrate.setEnabled(true);
						btn_CancelControllervibrate.setEnabled(true);
						btn_EnterPair1.setEnabled(true);
						btn_EnterPair2.setEnabled(true);
						btn_isEnterPair.setEnabled(true);
						btn_unbind.setEnabled(true);
						btn_shutdown.setEnabled(true);
	                    break;
	            }
	        }
	};
	@Override
	protected void onStart() {
		// TODO Auto-generated method stub
		super.onStart();
		//stopCVServcies();
	}
	@Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
		//stopCVServcies();
		
//		btn_StartStationUpdate.setEnabled(false);
//		btn_StartControllerUpdate.setEnabled(false);
//		btn_StartStm32Update.setEnabled(false);
//		btn_GetStationVersion.setEnabled(false);
//		btn_GetControllerVersion.setEnabled(false);
//		btn_Controllervibrate.setEnabled(false);
//		btn_CancelControllervibrate.setEnabled(false);
//		btn_EnterPair1.setEnabled(false);
//		btn_EnterPair2.setEnabled(false);
//		btn_isEnterPair.setEnabled(false);
//		btn_unbind.setEnabled(false);
//		btn_shutdown.setEnabled(false);
//		
//		new Thread(new Runnable() {
//            @Override
//            public void run() {
//                try {
//                    Thread.sleep(1000);//在子线程有一段耗时操作,比如请求网络
//                    mHandler_delay.sendEmptyMessage(0);
//                } catch (InterruptedException e) {
//                    e.printStackTrace();
//                }
//            }
//        }).start();
	}
	@Override
	protected void onPause() {
		// TODO Auto-generated method stub
		super.onPause();
		//startCVServcies();
	}
	@Override
	protected void onStop() {
		// TODO Auto-generated method stub
		super.onStop();
		//startCVServcies();
	}
	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		//startCVServcies();
	}
	
	private void startCVServcies()
	{
	    Intent intent = new Intent();  
	    intent.setAction(ACTION_CV_START_THREAD);  
	    this.sendBroadcast(intent);  
	}
	
	private void stopCVServcies()
	{
	    Intent intent = new Intent();  
	    intent.setAction(ACTION_CV_STOP_THREAD);  
	    this.sendBroadcast(intent);  
	}	
}
