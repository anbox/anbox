package org.anbox;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.os.SystemProperties;
import android.os.StrictMode;
import java.lang.Process;
import java.lang.ProcessBuilder;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import android.net.Uri;

import java.io.RandomAccessFile;
import java.io.FileWriter;
import java.io.OutputStreamWriter;
import java.io.FileOutputStream;
import java.io.File;

public class BootReceiver extends BroadcastReceiver {
    public BootReceiver() {
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        //start shashlikd app wide services
        MainApplication app = ((MainApplication) context.getApplicationContext());
        app.startServices();

        //Make a HTTP request
        //this does two things; fetches the name of the APK to run (bit overkill, when it could be a simple built prop)
        //informs the desktop side we're loaded, incase it needs to send anything with a fully operation system

        //Android generally blocks network in the main thread, but we're only going to be showing a black screen idling anyway.
        //may as well disable that setting
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder()
                .permitAll().build();
        StrictMode.setThreadPolicy(policy);

        //Get and install the APK if needed
        //if the APK doesn't exist the server will return a 403
        //10.0.2.2 is a special qemu setup for localhost
        try {
            Log.d("Shashlikd", "Fetching APK");
            URL url = new URL("http://10.0.2.2:60057/apk_file");
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.connect();
            int responseCode = conn.getResponseCode();
            if (responseCode == HttpURLConnection.HTTP_OK) {
                // opens input stream from the HTTP connection
                InputStream inputStream = conn.getInputStream();

                Log.d("Shashlikd", "Saving APK");
                // opens an output stream to save into file

                File outputDir = context.getCacheDir();
                File outputFile = File.createTempFile("apkSave", ".apk", outputDir);
                outputFile.setReadable(true, false);
                FileOutputStream outputStream = new FileOutputStream(outputFile);

                int bytesRead = -1;
                byte[] buffer = new byte[4096];
                while ((bytesRead = inputStream.read(buffer)) != -1) {
                    outputStream.write(buffer, 0, bytesRead);
                }

                outputStream.close();
                inputStream.close();

                Log.d("Shashlikd", "File downloaded");
                Log.d("Shashlikd", outputFile.getPath());

                Log.d("Shashlikd", "Installing APK");

                Process result = new ProcessBuilder()
                            .command("pm", "install", "-r", "-d", outputFile.getPath())
                            .start();
                result.waitFor();
                Log.d("Shashlikd", "Apk install finished with " + result.exitValue());
            }
        } catch (MalformedURLException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }


        //Find the name of the APK to run and start the main activity
        try {
            Log.d("Shashlikd", "Getting APK ID");
            URL url = new URL("http://10.0.2.2:60057/startup");
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();

            conn.connect();
            InputStream inputStream = conn.getInputStream();
            BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, "UTF-8"));
            String apk = reader.readLine();

            if (!apk.isEmpty()) {
                //now try to start that APK
                Log.d("Shashlikd", "STARTING");
                Intent launchIntent = context.getPackageManager().getLaunchIntentForPackage(apk);
                if (launchIntent != null) {
                    context.startActivity(launchIntent);
                } else {
                    Log.e("Shashlikd", "APK not installed");
                }
            } else {
                Log.w("Shashlikd", "No APK name returned");
            }
        } catch (MalformedURLException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
