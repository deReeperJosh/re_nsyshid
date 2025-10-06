import React, { useState, useEffect } from "react";
import { useNavigate } from "react-router-dom";

export default function App() {
  const [ip, setIp] = useState("");
  const [status, setStatus] = useState(null);
  const [inputValue, setInputValue] = useState("");
  const navigate = useNavigate();

  useEffect(() => {
    const savedIp = localStorage.getItem("deviceIp");
    if (savedIp) {
      setIp(savedIp);
      setInputValue(savedIp);
    }
  }, []);

  const handleSave = async () => {
    if (!inputValue) return alert("Please enter an IP address.");
    localStorage.setItem("deviceIp", inputValue);
    setIp(inputValue);
    await testConnection(inputValue);
  };

  const testConnection = async (ipAddress) => {
    setStatus("⏳ Checking server...");
    try {
      const response = await fetch(`http://${ipAddress}:8853`, { method: "GET" });
      if (response.ok) {
        setStatus("✅ Server is running!");
        setTimeout(() => navigate("/dashboard"), 1000);
      } else {
        setStatus("⚠️ Server responded but not OK.");
      }
    } catch (err) {
      console.error(err.message);
      setStatus("❌ Could not reach the server.");
    }
  };

  return (
    <div className="min-h-screen flex flex-col items-center justify-center bg-gray-100 p-6">
      <div className="bg-white rounded-2xl shadow-lg p-8 w-full max-w-md">
        <h1 className="text-2xl font-bold mb-4 text-center">Local Server Checker</h1>
        <label className="block text-sm font-medium mb-2">Device IP Address:</label>
        <input
          type="text"
          value={inputValue}
          onChange={(e) => setInputValue(e.target.value)}
          placeholder="e.g. 192.168.1.10"
          className="border border-gray-300 rounded-xl p-2 w-full mb-4"
        />
        <button
          onClick={handleSave}
          className="bg-blue-600 text-white px-4 py-2 rounded-xl w-full hover:bg-blue-700"
        >
          Save & Check Server
        </button>

        {status && (
          <div className="mt-4 text-center text-lg font-semibold">
            {status}
          </div>
        )}

        {ip && (
          <div className="mt-2 text-center text-gray-500 text-sm">
            Saved IP: {ip}
          </div>
        )}
      </div>
    </div>
  );
}
