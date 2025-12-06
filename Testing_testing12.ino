#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <WebServer.h>

// ===== Wi-Fi Credentials =====
const char* ssid = "WI-FI name"; //Your WI-FI name
const char* password = "WI-FI password"; //Your WI-FI password

// ===== SMTP (Gmail) Settings =====
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

const char* emailSenderAccount = "Your Email";  // Your Gmail
const char* emailSenderPassword = "****************";     // 16-character App Password
const char* emailRecipient = "Recipient's Email'";   // Recipient

// ===== Web Server =====
WebServer server(80);

// HTML form for the Web-server UI
const char* htmlForm = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>ESP32 Email Sender</title>
  </head>
  <body>
    <h1>Send Email</h1>
    <form action="/send" method="POST">
      <label>Subject:</label><br>
      <input type="text" name="subject"><br>
      <label>Message:</label><br>
      <textarea name="message" rows="5" cols="30"></textarea><br><br>
      <input type="submit" value="Send">
    </form>
  </body>
</html>
)rawliteral";

// ===== Functions =====
void sendEmail(const String &subject, const String &message) {
  Serial.println("Preparing email...");

  SMTPSession smtp;

  SMTP_Message email;
  email.sender.name = "ESP32 Mailer";
  email.sender.email = emailSenderAccount;
  email.subject = subject.c_str();
  email.addRecipient("Recipient", emailRecipient);
  email.addRecipient("Recepient 2", emailRecipient);
  email.text.content = message.c_str();

  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = emailSenderAccount;
  session.login.password = emailSenderPassword;
  session.login.user_domain = "";

  Serial.println("Connecting to SMTP server...");
  if (!smtp.connect(&session)) {
    Serial.println("SMTP Connect failed!");
    return;
  }

  Serial.println("Sending email...");
  if (!MailClient.sendMail(&smtp, &email)) {
    Serial.print("Error sending Email: ");
    Serial.println(smtp.errorReason());
  } else {
    Serial.println("Email sent successfully!");
  }

  smtp.closeSession();
}

// Web server handlers
void handleRoot() {
  server.send(200, "text/html", htmlForm);
}

void handleSend() {
  String subject = server.arg("subject");
  String message = server.arg("message");

  if (subject.length() > 0 && message.length() > 0) {
    sendEmail(subject, message);
    server.send(200, "text/html", "<h2>Email sent!</h2><a href='/'>Go Back</a>");
  } else {
    server.send(400, "text/html", "<h2>Missing subject or message!</h2><a href='/'>Go Back</a>");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Allow Serial to initialize
  Serial.println("ESP32 starting...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to Wi-Fi!");
    return;
  }
  Serial.println("\nConnected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/send", HTTP_POST, handleSend);
  server.begin();
  Serial.println("HTTP server started. Open your browser and go to the ESP32 IP to send email.");
}

void loop() {
  server.handleClient();
}
