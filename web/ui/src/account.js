
facefull.doEventHandlerAttach("onLogin", function(str) {
    let data = JSON.parse(str);
    console.log("onLogin", data);
    AlertHideCustom("NC");
    switch (data.state) {
        case 0:
            AlertShow("Login error", "Please check your login data and try again.", "error");
            break;
        case 1:
            document.getElementById("UAN").innerHTML = data.username;
            AlertHideCustom("AL");
            break;
        case 2:
            AlertShow("Login error", "You do not have an active license for this product. Please contact NickWare Group technical support.", "error");
            break;
        default:
        case -1:
            AlertShow("Login error", "Network error. Please try again later or contact NickWare Group technical support.", "error");
            break;
    }
});

function doTryLogin() {
    let login = document.getElementById("IAL").value;
    let password = document.getElementById("IAP").value;

    let data = {
        login: login,
        password: password
    }

    AlertShowCustom("NC");
    facefull.doEventSend("doTryLogin", JSON.stringify(data));
}

function doLogout() {
    AlertShow("Logout", "Do you really want to logout?", "warning", "YESNO", [function() {
        facefull.doEventSend("doLogout");
        document.getElementById("UAN").innerHTML = "";
        AlertShowCustom("AL");
    }, function() {
    }]);
}
