
function _refresh() {
    let xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = () => {
        if (xhttp.readyState == 4 && xhttp.status == 200) {
            let data = JSON.parse(xhttp.responseText);
            let table = document.getElementById("messages");
            table.innerHTML = "";

            for (let i = 0; i < data.messages.length; i++) {
                let _div = document.createElement("DIV");
                _div.className = "message";
                _div.innerText = data.messages[i];
                table.appendChild(_div);
            }
        }
    };
    xhttp.open("GET", "/messages", true);
    xhttp.send();
}

function _post() {
    if (msg.value == "") return;

    let xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = () => {
        if (xhttp.readyState == 4 && xhttp.status == 201)
            _refresh();
    };
    xhttp.open("POST", "/messages", true);
    xhttp.setRequestHeader("Content-Type", "application/json");
    xhttp.send(JSON.stringify({
        message: msg.value
    }));
    msg.value = "";
}

function _shutdown() {
    location.href = "/shutdown";
}

_refresh();
