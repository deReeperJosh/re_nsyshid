"use strict";

(() => {
    const loginForm = document.getElementById("login-form");
    const registerForm = document.getElementById("register-form");

    const formErrorBanner = document.getElementById("form-error");

    const tabs = {
        login:    document.getElementById("tab:login"),
        register: document.getElementById("tab:register")
    };

    const tabContents = {
        login:    loginForm,
        register: registerForm,
    };

    function resetForms() {
        loginForm.reset();
        registerForm.reset();
        setFormError(undefined);
    }

    let currentTab = 'login';
    function switchTab(name) {
        if (name === currentTab)
            return;

        currentTab = name;

        resetForms();

        for (const key in tabs) {
            if (key === name) {
                tabs[key].className = "active";
                tabContents[key].className = "tab-controlled active";
                continue;
            }

            tabs[key].className = "";
            tabContents[key].className = "tab-controlled";
        }
    }

    for (const key in tabs)
        tabs[key].onclick = () => switchTab(key);
    
    function postFormAsJson(f, url, validator) {
        const resObj = {};
        const inputs = f.getElementsByTagName("INPUT");
        
        for (let i = 0; i < inputs.length; i++) {
            let e = inputs[i];
            resObj[e.name] = e.value;
        }

        if (typeof(validator) === 'function' && !validator(resObj))
            return;

        fetch(url, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(resObj)
        }).then(response => {
            if (response.status !== 200) {
                setFormError(`Server responded: ${response.status} ${response.statusText}`);
                return undefined;
            }

            return response.json();
        }).then(data => {
            if (data && 'error' in data && data.error) {
                setFormError(`Server error: ${data.error}`);
                return;
            }
            location.href = "/home";
        });
    }

    function setFormError(message) {
        if (message) {
            console.error("Form validation error:", message);
            formErrorBanner.style.display = "block";
            formErrorBanner.innerText = message;
            return;
        }

        formErrorBanner.style.display = "none";
    }

    function loginFormValidator(data) {
        if (data.username.length === 0) {
            setFormError("Username should not be empty");
            return false;
        }

        if (data.password.length === 0) {
            setFormError("Password should not be empty");
            return false;
        }

        return true;
    }

    function registerFormValidator(data) {
        if (data.username.length < 3) {
            setFormError("Username should be at least 3 characters long");
            return false;
        }

        if (data.password1.length < 8) {
            setFormError("Password should be at least 8 characters long");
            return false;
        }

        if (data.password2 != data.password2) {
            setFormError("The passwords should be equal");
            return false;
        }

        if (data.displayname.length === 0) {
            setFormError("Displayname should not be empty");
            return false;
        }

        return true;
    }

    loginForm.onsubmit = () => {
        postFormAsJson(loginForm, "/login", loginFormValidator);
        return false;
    };
    registerForm.onsubmit = () => {
        try { postFormAsJson(registerForm, "/register", registerFormValidator); }
        catch (e) { console.log(e); }
        return false;
    };
})();
