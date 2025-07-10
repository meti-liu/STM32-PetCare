package com.example.deepseekbridge;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "DeepSeekBridge";
    private static final int REQUEST_ENABLE_BT = 1;
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB"); // 标准串口服务UUID

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket bluetoothSocket;
    private OutputStream outputStream;
    private InputStream inputStream;
    private Thread workerThread;
    private byte[] readBuffer;
    private int readBufferPosition;
    private volatile boolean stopWorker;

    private TextView tvStatus;
    private TextView tvResponse;
    private EditText etQuery;
    private Button btnConnect;
    private Button btnSend;

    private String deviceAddress = "00:00:00:00:00:00"; // 替换为HC05的MAC地址
    private static final String DEEPSEEK_API_KEY = "your_api_key_here"; // 替换为你的DeepSeek API密钥

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tvStatus = findViewById(R.id.tvStatus);
        tvResponse = findViewById(R.id.tvResponse);
        etQuery = findViewById(R.id.etQuery);
        btnConnect = findViewById(R.id.btnConnect);
        btnSend = findViewById(R.id.btnSend);

        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            Toast.makeText(this, "设备不支持蓝牙", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }

        btnConnect.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                connectBluetooth();
            }
        });

        btnSend.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String query = etQuery.getText().toString().trim();
                if (!query.isEmpty()) {
                    sendDeepSeekQuery(query);
                }
            }
        });
    }

    private void connectBluetooth() {
        if (!bluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.BLUETOOTH_CONNECT}, REQUEST_ENABLE_BT);
                return;
            }
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            return;
        }

        tvStatus.setText("正在连接...");

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    // 获取已配对设备
                    if (ActivityCompat.checkSelfPermission(MainActivity.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.BLUETOOTH_CONNECT}, REQUEST_ENABLE_BT);
                                tvStatus.setText("需要蓝牙权限");
                            }
                        });
                        return;
                    }

                    Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
                    BluetoothDevice device = null;

                    for (BluetoothDevice pairedDevice : pairedDevices) {
                        if (pairedDevice.getAddress().equals(deviceAddress)) {
                            device = pairedDevice;
                            break;
                        }
                    }

                    if (device == null) {
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                tvStatus.setText("未找到HC05设备");
                            }
                        });
                        return;
                    }

                    // 连接设备
                    bluetoothSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
                    bluetoothSocket.connect();
                    outputStream = bluetoothSocket.getOutputStream();
                    inputStream = bluetoothSocket.getInputStream();

                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            tvStatus.setText("已连接到HC05");
                            btnConnect.setEnabled(false);
                            btnSend.setEnabled(true);
                        }
                    });

                    // 开始监听蓝牙数据
                    beginListenForData();

                } catch (IOException e) {
                    final String errorMsg = e.getMessage();
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            tvStatus.setText("连接失败: " + errorMsg);
                        }
                    });
                    try {
                        if (bluetoothSocket != null) {
                            bluetoothSocket.close();
                        }
                    } catch (IOException closeException) {
                        Log.e(TAG, "关闭蓝牙套接字失败", closeException);
                    }
                }
            }
        }).start();
    }

    private void beginListenForData() {
        final Handler handler = new Handler(Looper.getMainLooper());
        stopWorker = false;
        readBufferPosition = 0;
        readBuffer = new byte[1024];

        workerThread = new Thread(new Runnable() {
            @Override
            public void run() {
                while (!Thread.currentThread().isInterrupted() && !stopWorker) {
                    try {
                        int bytesAvailable = inputStream.available();
                        if (bytesAvailable > 0) {
                            byte[] packetBytes = new byte[bytesAvailable];
                            inputStream.read(packetBytes);

                            for (int i = 0; i < bytesAvailable; i++) {
                                byte b = packetBytes[i];
                                if (b == '\n') {
                                    byte[] encodedBytes = new byte[readBufferPosition];
                                    System.arraycopy(readBuffer, 0, encodedBytes, 0, encodedBytes.length);
                                    final String data = new String(encodedBytes, "UTF-8");
                                    readBufferPosition = 0;

                                    handler.post(new Runnable() {
                                        @Override
                                        public void run() {
                                            // 处理接收到的数据
                                            processReceivedData(data);
                                        }
                                    });
                                } else {
                                    readBuffer[readBufferPosition++] = b;
                                }
                            }
                        }
                    } catch (IOException ex) {
                        stopWorker = true;
                    }
                }
            }
        });

        workerThread.start();
    }

    private void processReceivedData(String data) {
        Log.d(TAG, "收到数据: " + data);

        // 检查是否是DeepSeek查询请求
        if (data.startsWith("DeepSeek Query: ")) {
            String query = data.substring("DeepSeek Query: ".length()).trim();
            tvStatus.setText("收到查询: " + query);
            callDeepSeekAPI(query);
        }
    }

    private void sendDeepSeekQuery(String query) {
        if (outputStream != null) {
            try {
                String command = "+deepseek " + query + "\r\n";
                outputStream.write(command.getBytes());
                tvStatus.setText("已发送查询: " + query);
            } catch (IOException e) {
                tvStatus.setText("发送失败: " + e.getMessage());
            }
        } else {
            tvStatus.setText("未连接到蓝牙设备");
        }
    }

    private void callDeepSeekAPI(final String query) {
        new AsyncTask<Void, Void, String>() {
            @Override
            protected String doInBackground(Void... voids) {
                try {
                    URL url = new URL("https://api.deepseek.com/chat/completions");
                    HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                    conn.setRequestMethod("POST");
                    conn.setRequestProperty("Content-Type", "application/json");
                    conn.setRequestProperty("Authorization", "Bearer " + DEEPSEEK_API_KEY);
                    conn.setDoOutput(true);

                    JSONObject jsonRequest = new JSONObject();
                    jsonRequest.put("model", "deepseek-chat");

                    JSONArray messages = new JSONArray();
                    JSONObject systemMessage = new JSONObject();
                    systemMessage.put("role", "system");
                    systemMessage.put("content", "你是一个有用的助手。");
                    messages.put(systemMessage);

                    JSONObject userMessage = new JSONObject();
                    userMessage.put("role", "user");
                    userMessage.put("content", query);
                    messages.put(userMessage);

                    jsonRequest.put("messages", messages);
                    jsonRequest.put("stream", false);

                    OutputStream os = conn.getOutputStream();
                    os.write(jsonRequest.toString().getBytes("UTF-8"));
                    os.close();

                    int responseCode = conn.getResponseCode();
                    if (responseCode == HttpURLConnection.HTTP_OK) {
                        BufferedReader in = new BufferedReader(new InputStreamReader(conn.getInputStream()));
                        String inputLine;
                        StringBuilder response = new StringBuilder();

                        while ((inputLine = in.readLine()) != null) {
                            response.append(inputLine);
                        }
                        in.close();

                        JSONObject jsonResponse = new JSONObject(response.toString());
                        JSONArray choices = jsonResponse.getJSONArray("choices");
                        JSONObject choice = choices.getJSONObject(0);
                        JSONObject message = choice.getJSONObject("message");
                        return message.getString("content");
                    } else {
                        return "API调用失败: " + responseCode;
                    }
                } catch (IOException | JSONException e) {
                    return "API调用异常: " + e.getMessage();
                }
            }

            @Override
            protected void onPostExecute(String result) {
                tvResponse.setText(result);
                sendDeepSeekResponse(result);
            }
        }.execute();
    }

    private void sendDeepSeekResponse(String response) {
        if (outputStream != null) {
            try {
                String command = "+deepseek_response " + response + "\r\n";
                outputStream.write(command.getBytes());
                tvStatus.setText("已发送响应");
            } catch (IOException e) {
                tvStatus.setText("发送响应失败: " + e.getMessage());
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (workerThread != null) {
            stopWorker = true;
            workerThread.interrupt();
        }

        try {
            if (outputStream != null) {
                outputStream.close();
            }
            if (inputStream != null) {
                inputStream.close();
            }
            if (bluetoothSocket != null) {
                bluetoothSocket.close();
            }
        } catch (IOException e) {
            Log.e(TAG, "关闭连接失败", e);
        }
    }
}