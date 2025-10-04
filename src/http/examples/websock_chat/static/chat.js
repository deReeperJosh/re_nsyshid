"use strict";

(() => {
    const socket = new WebSocket("ws://localhost:8088/ws");

    socket.onclose = () => {
        console.error("Good bye, remote socket :(");
        alert("WebSocket disconnected :(");
    };

    socket.addEventListener("open", function (event) {
        chat_init();
    });

    socket.addEventListener("message", function (event) {
        let data = JSON.parse(event.data);
        
        if (data.type === "text_message") {
            addMessageToContainer(data.sender.name, data.sender.id, new Date(data.time * 1000).toLocaleString(), data.content);
        }
    });

    const message_container = document.getElementById("messages");
    const message_input     = document.getElementById("message-box");
    let prevId = null;

    function addMessageToContainer(senderName, senderId, timeString, content) {
        let div = document.createElement("DIV");
        div.className = `message ${senderId === "$self" ? "self" : ""}`;

        if (prevId !== senderId) {
            let senderNameAndTime = document.createElement("SMALL");
            senderNameAndTime.innerText = `${senderName} ~ ${timeString}`;
            div.appendChild(senderNameAndTime);
        }

        prevId = senderId;

        let messageContent = document.createElement("P");
        messageContent.innerText = content;
        div.appendChild(messageContent);

        message_container.appendChild(div);
        message_container.scrollBy(0,9999);
        setTimeout(() => { messageContent.className += " animate" }, 5);
    }

    function sendCurrentMessage() {
        let body = message_input.value.trim();

        message_input.value = "";

        if (!body)
            return;

        socket.send(JSON.stringify({
            message: body
        }));
    }
    
    function chat_init() {
        console.log("We are connected!");

        message_input.onkeydown = (e) => {
            if (e.key === "Enter")
                sendCurrentMessage();
        };

        document.getElementById("send-button").onclick = () => sendCurrentMessage();
    }
})();
