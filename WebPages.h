/*
  DOC STRING
*/
#ifndef WebPages_h
#define WebPages_h

String homePage = R"END(
<!DOCTYPE html>
  <head>
    <title>WiFi Setup Home</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html,
      body {
        margin: 0;
        border: 0;
        padding: 0;
        background-color: teal;
      }

      h1,
      form {
        text-align: center;
      }

      form {
        padding: 0.9rem;
      }

      div {
        margin: 0 3rem 0 3rem;
        border-radius: 1rem;
        background-color: lightgray;
      }

      /*Buttons*/

      button {
        padding: 1rem;
        font-size: 2rem;
        box-shadow: 10px 10px black;
        border-radius: 1rem;
      }

      button:active {
        box-shadow: 0px 0px;
        transform: translate(10px, 10px);
      }

      button#to-setup {
        background-color: red;
      }

      button#to-app {
        background-color: blue;
        color: white;
      }

      button#show-info {
        background-color: yellow
      }
    </style>
  </head>
  <body>
    <h1>WiFiSetup Home Page</h1>
    <div id="menu-container">
      <form action="/app">
        <button type="submit" id="to-app">To App</button>
      </form>
      <form action="/wifi-info">
        <button type="submit" id="show-info">Show WiFi Info</button>
      </form>
      <form action="/connect_page">
        <button type="submit" id="to-setup">To Setup </button>
      </form>
    </div>
  </body>
</html>
)END";

String networkSetupPage = R"END(
<!DOCTYPE html>
<html>
  <head>
    <title>Network Setup Page</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
  </head>
  <style>
    html
    body {
      magrin: 0;
      padding: 0;
      background-color: green;
    }

    h1 {
      padding: 1rem;
      border-radius: 10rem;
      margin: 1rem;
      background-color: limegreen;
      text-align: center;
    }

    div#container {
      padding: 1rem;
      border-radius: 1rem;
      background-color: lightblue;
      text-align: center;
      height: 50%;
    }

    input#submit-button {
      padding: 1rem;
      margin: 1rem;
      box-shadow: 10px 10px;
      border-radius: 1rem;
      background-color: yellowgreen;
    }

    input#submit-button:active {
      box-shadow: 0 0;
      transform: translate(10px, 10px);
    }

    select#names {
      padding: 1rem;
      margin: 1rem;
    }
  </style>
  <body>
    <h1>Select a WiFi Network</h1>
    <div id="container">
      <form action="/connect">
        <label for="names">Networks:</label>
        <select name="ssid" id="names">
          <option>
        </select>
        <br>
        <label for="password">Password:</label>
        <input type="password" name="pass" id="password" required><br>
        <input type="checkbox" onclick="showPass()"><span>Show Password</span>
        <input id="submit-button" type="submit" action="submit" value="Connect">
      </form>
    </div>
    <script>
      var checkBox = document.getElementById("password");
      
      function showPass() {
        if (checkBox.type === "password") {
          checkBox.type = "text";
        } else {
          checkBox.type = "password";
        }
      }
    </script>
  </body>
</html>
)END";

String successPage = R"END(
<!DOCTYPE html>
<html>
  <head>
    <title>Lamp control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
  </head>
  <body>
    <h1>Success!<h1>
    <p>You have successfully logged into ^networkname^</p>
  </body>
</html>
)END";

String badPassPage = R"END(
<!DOCTYPE html>
<html>
  <head>
    <title>Bad Password</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html,
      body {
        background-color: red;
      }

      h1 {
        text-align: center;
        background-color: yellow;
        border-radius: 10px;
      }

      p {
        margin-top: 5rem;
        text-align: center;
        font-size: 20px;
      }

      a {
        margin-top: 5rem;
        padding: 1rem;
        background-color: green;
        text-decoration: none;
        color: black;
        border-radius: 1rem;
        display: inline-block;
        box-shadow: 10px 10px;
      }

      div {
        text-align: center;
      }

      a:active {
        box-shadow: 0 0;
        transform: translate(10px, 10px);
      }
    </style>
  </head>
  <body>
    <h1 id="warning_text">Wrong Password for ^networkname^</h1>

    <p>
      Click the button below to return to the setup page.
    </p>

    <div>
      <a href="/">Back to Setup</a>
    </div>
    <script>
      var warningText = document.querySelector("#warning_text");

      var blinkH1 = setInterval(function() {
        if (warningText.style.visibility === "hidden") {
          warningText.style.visibility = "visible";
        } else {
          warningText.style.visibility = "hidden";
        }
      }, 1000);
    </script>
  </body>
</html>
)END";
#endif