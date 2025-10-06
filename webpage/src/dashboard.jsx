import React, { useState, useEffect } from "react";
import { useNavigate } from "react-router-dom";

export default function Dashboard() {
    const [ip, setIp] = useState("");
    const [status, setStatus] = useState(null);
    const [device, setDevice] = useState(null);
    const [loading, setLoading] = useState(true);
    const [message, setMessage] = useState("");
    const [selectedDevice, setSelectedDevice] = useState("none");
    const [deviceData, setDeviceData] = useState({});
    const [fileBrowserVisible, setFileBrowserVisible] = useState(false);
    const [currentFiles, setCurrentFiles] = useState([]);
    const [currentPath, setCurrentPath] = useState("");
    const [activeSlot, setActiveSlot] = useState(null);
    const [loadingFiles, setLoadingFiles] = useState(false);
    const [createModalVisible, setCreateModalVisible] = useState(false);
    const [createId, setCreateId] = useState("");
    const [createVar, setCreateVar] = useState("");

    const navigate = useNavigate();

    const deviceNames = {
        0: "none",
        1: "skylander",
        2: "infinity",
        3: "dimensions",
    };

    const deviceDisplayNames = {
        none: "None",
        skylander: "Skylander Portal",
        infinity: "Infinity Base",
        dimensions: "Dimensions Toypad",
    };

    // Fetch status from server
    const fetchStatus = async (ipAddress) => {
        setLoading(true);
        try {
            const res = await fetch(`http://${ipAddress}:8853/status`);
            if (!res.ok) throw new Error("Server error");
            const data = await res.json();
            setStatus(data.status);
            setDevice(data.device);
            setSelectedDevice(deviceNames[data.device] || "none");

            if (data.device !== 0) fetchDeviceData(ipAddress, deviceNames[data.device]);
            setMessage("");
        } catch (e) {
            setMessage("❌ Could not fetch status from the server.");
            setDeviceData({});
        } finally {
            setLoading(false);
        }
    };

    const fetchDeviceData = async (ipAddress, deviceType) => {
        try {
            const res = await fetch(`http://${ipAddress}:8853/device/${deviceType}`);
            if (!res.ok) throw new Error("Server error");
            const data = await res.json();
            setDeviceData(data);
        } catch (e) {
            setDeviceData({});
            setMessage(`⚠️ Failed to fetch ${deviceDisplayNames[deviceType]} data.`);
        }
    };

    const toggleEmulation = async (enable) => {
        try {
            const endpoint = enable ? "enable" : "disable";
            await fetch(`http://${ip}:8853/status/${endpoint}`, { method: "POST" });
            await fetchStatus(ip);
            setMessage(enable ? "✅ Emulation enabled" : "⛔ Emulation disabled");
        } catch {
            setMessage("⚠️ Failed to toggle emulation.");
        }
    };

    const updateDeviceType = async (type) => {
        try {
            await fetch(`http://${ip}:8853/status/device`, {
                method: "POST",
                headers: { "Content-Type": "text/plain" },
                body: type,
            });
            await fetchStatus(ip);
            setMessage(`✅ Device set to ${deviceDisplayNames[type]}`);
        } catch {
            setMessage("⚠️ Failed to change device.");
        }
    };

    const handleDeviceChange = (e) => {
        const type = e.target.value;
        setSelectedDevice(type);
        updateDeviceType(type);
    };

    const handleDisconnect = () => {
        localStorage.removeItem("deviceIp");
        navigate("/");
    };

    const removeSlot = async (deviceType, slotNumber) => {
        try {
            await fetch(`http://${ip}:8853/device/${deviceType}/remove`, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ slot: slotNumber }),
            });
            setMessage(`✅ Removed slot ${slotNumber}`);
            fetchStatus(ip);
        } catch {
            setMessage(`⚠️ Failed to remove slot ${slotNumber}`);
        }
    };

    const openFileBrowser = async (deviceType, slot) => {
        setActiveSlot(slot);
        setCurrentPath("");
        setFileBrowserVisible(true);
        fetchFiles("");
    };

    const fetchFiles = async (path) => {
        setLoadingFiles(true);
        try {
            const response = await fetch(`http://${ip}:8853/files?path=${(path)}`);
            const data = await response.json();
            setCurrentFiles(data.files);
            setCurrentPath(path);
        } catch (err) {
            setMessage("⚠️ Failed to fetch files");
            setCurrentFiles([]);
        } finally {
            setLoadingFiles(false);
        }
    };

    const handleFileClick = (fileName) => {
        if (fileName.endsWith("/")) {
            const newPath = currentPath + fileName;
            fetchFiles(newPath);
        } else {
            loadFileIntoSlot(activeSlot, currentPath + fileName);
        }
    };

    const loadFileIntoSlot = async (slot, filePath) => {
        try {
            await fetch(`http://${ip}:8853/device/${selectedDevice}/load`, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ slot, file: filePath }),
            });
            setMessage(`✅ Loaded ${filePath} into slot ${slot}`);
            fetchStatus(ip);
            setFileBrowserVisible(false);
        } catch {
            setMessage(`⚠️ Failed to load file`);
        }
    };

    // 🟢 CREATE FEATURE
    const openCreateModal = (slot) => {
        setActiveSlot(slot);
        setCreateId("");
        setCreateVar("");
        setCreateModalVisible(true);
    };

    const handleCreate = async () => {
        const idVal = parseInt(createId);
        const varVal = parseInt(createVar || 0);

        // Validate inputs
        if (selectedDevice === "skylander") {
            if (isNaN(idVal) || idVal < 0 || idVal > 65535) {
                setMessage("⚠️ ID must be between 0 and 65535");
                return;
            }
            if (isNaN(varVal) || varVal < 0 || varVal > 65535) {
                setMessage("⚠️ Variant must be between 0 and 65535");
                return;
            }
        } else {
            if (isNaN(idVal) || idVal < 0 || idVal > 4294967295) {
                setMessage("⚠️ ID must be between 0 and 4294967295");
                return;
            }
        }

        try {
            const payload =
                selectedDevice === "skylander"
                    ? { id: idVal, var: varVal }
                    : { id: idVal };

            const res = await fetch(`http://${ip}:8853/device/${selectedDevice}/create`, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload),
            });

            if (!res.ok) throw new Error("Failed to create device file");
            const data = await res.json();

            if (data.file) {
                await loadFileIntoSlot(activeSlot, data.file);
                setMessage(`✅ Created and loaded new file into slot ${activeSlot}`);
            } else {
                setMessage("⚠️ File creation returned no file path");
            }

            setCreateModalVisible(false);
        } catch {
            setMessage("⚠️ Failed to create file");
        }
    };

    useEffect(() => {
        const savedIp = localStorage.getItem("deviceIp");
        if (!savedIp) {
            navigate("/");
            return;
        }
        setIp(savedIp);
        fetchStatus(savedIp);
    }, []);

    return (
        <div className="min-h-screen flex flex-col items-center justify-center bg-gray-100 p-6 relative">
            <div className="bg-white rounded-2xl shadow-lg p-8 w-full max-w-6xl">
                <div className="flex justify-between items-center mb-6">
                    <h1 className="text-2xl font-bold">Emulation Dashboard</h1>
                    <div className="flex gap-4">
                        <button
                            onClick={() => fetchStatus(ip)}
                            className="px-4 py-2 bg-blue-600 text-white rounded-xl hover:bg-blue-700 transition"
                        >
                            Refresh
                        </button>
                        <button
                            onClick={handleDisconnect}
                            className="px-4 py-2 bg-red-500 text-white rounded-xl hover:bg-red-600 transition"
                        >
                            Disconnect
                        </button>
                    </div>
                </div>

                {loading ? (
                    <p className="text-center text-gray-600">⏳ Loading status...</p>
                ) : (
                    <div className="flex flex-col lg:flex-row gap-6">
                        {/* Left Panel */}
                        <div className="flex-1 bg-gray-50 p-6 rounded-xl shadow-md">
                            <p className="text-gray-700 mb-2">
                                Connected to: <strong>{ip}</strong>
                            </p>
                            <p className="text-gray-700 mb-2">
                                Emulation Status:{" "}
                                <strong
                                    className={status === 1 ? "text-green-600" : "text-red-600"}
                                >
                                    {status === 1 ? "Enabled" : "Disabled"}
                                </strong>
                            </p>
                            <p className="text-gray-700 mb-4">
                                Device: <strong>{deviceDisplayNames[selectedDevice]}</strong>
                            </p>

                            <div className="flex flex-col sm:flex-row gap-3 mb-6">
                                <button
                                    onClick={() => toggleEmulation(true)}
                                    disabled={status === 1}
                                    className={`w-full px-4 py-2 rounded-xl text-white font-medium transition ${status === 1
                                            ? "bg-gray-400 cursor-not-allowed"
                                            : "bg-green-600 hover:bg-green-700"
                                        }`}
                                >
                                    Enable Emulation
                                </button>
                                <button
                                    onClick={() => toggleEmulation(false)}
                                    disabled={status === 0}
                                    className={`w-full px-4 py-2 rounded-xl text-white font-medium transition ${status === 0
                                            ? "bg-gray-400 cursor-not-allowed"
                                            : "bg-red-600 hover:bg-red-700"
                                        }`}
                                >
                                    Disable Emulation
                                </button>
                            </div>

                            <div className="mb-6">
                                <label className="block text-sm font-medium mb-2">
                                    Select Device Type
                                </label>
                                <select
                                    value={selectedDevice}
                                    onChange={handleDeviceChange}
                                    className="border border-gray-300 rounded-xl p-2 w-full text-gray-800 focus:outline-none focus:ring-2 focus:ring-blue-500"
                                >
                                    <option value="none">None</option>
                                    <option value="skylander">Skylander Portal</option>
                                    <option value="infinity">Infinity Base</option>
                                    <option value="dimensions">Dimensions Toypad</option>
                                </select>
                            </div>

                            {message && (
                                <div className="text-center text-sm font-medium text-gray-700 mt-4">
                                    {message}
                                </div>
                            )}
                        </div>

                        {/* Right Panel */}
                        {device !== 0 && (
                            <div className="flex-1 bg-gray-50 p-6 rounded-xl shadow-md overflow-auto max-h-[500px]">
                                <h2 className="text-xl font-semibold mb-4">
                                    {deviceDisplayNames[selectedDevice]} Data
                                </h2>
                                <table className="w-full text-left border-collapse">
                                    <thead>
                                        <tr className="border-b border-gray-300">
                                            <th className="p-2">Slot</th>
                                            <th className="p-2">Name</th>
                                            <th className="p-2">Actions</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                        {Object.keys(deviceData)
                                            .map(Number)
                                            .sort((a, b) => a - b)
                                            .map((key) => {
                                                const slot = deviceData[key];
                                                return (
                                                    <tr key={key} className="border-b border-gray-200">
                                                        <td className="p-2">{key}</td>
                                                        <td className="p-2">{slot.name}</td>
                                                        <td className="p-2 space-x-2">
                                                            <button
                                                                onClick={() => openCreateModal(key)}
                                                                className="px-3 py-1 bg-green-500 text-white rounded-xl hover:bg-green-600 transition text-sm"
                                                            >
                                                                Create
                                                            </button>
                                                            <button
                                                                onClick={() => openFileBrowser(selectedDevice, key)}
                                                                className="px-3 py-1 bg-blue-500 text-white rounded-xl hover:bg-blue-600 transition text-sm"
                                                            >
                                                                Load
                                                            </button>
                                                            <button
                                                                onClick={() => removeSlot(selectedDevice, key)}
                                                                className="px-3 py-1 bg-red-500 text-white rounded-xl hover:bg-red-600 transition text-sm"
                                                            >
                                                                Remove
                                                            </button>
                                                        </td>
                                                    </tr>
                                                );
                                            })}
                                    </tbody>
                                </table>
                            </div>
                        )}
                    </div>
                )}
            </div>

            {/* 🟢 CREATE MODAL */}
            {createModalVisible && (
                <div className="fixed inset-0 bg-black bg-opacity-40 flex items-center justify-center z-50">
                    <div className="bg-white rounded-xl p-6 shadow-2xl w-96">
                        <h3 className="text-lg font-bold mb-4 text-center">
                            Create New {deviceDisplayNames[selectedDevice]}
                        </h3>

                        <p className="text-sm text-center mb-4">
                            Creating for slot <strong>{activeSlot}</strong>
                        </p>

                        <div className="space-y-3">
                            <div>
                                <label className="block text-sm font-medium mb-1">ID</label>
                                <input
                                    type="number"
                                    value={createId}
                                    onChange={(e) => setCreateId(e.target.value)}
                                    className="w-full border rounded-lg p-2 focus:ring-2 focus:ring-blue-500"
                                    placeholder="Enter ID"
                                />
                            </div>

                            {selectedDevice === "skylander" && (
                                <div>
                                    <label className="block text-sm font-medium mb-1">
                                        Variant
                                    </label>
                                    <input
                                        type="number"
                                        value={createVar}
                                        onChange={(e) => setCreateVar(e.target.value)}
                                        className="w-full border rounded-lg p-2 focus:ring-2 focus:ring-blue-500"
                                        placeholder="Enter Variant"
                                    />
                                </div>
                            )}
                        </div>

                        <div className="flex justify-between mt-6">
                            <button
                                onClick={() => setCreateModalVisible(false)}
                                className="px-4 py-2 bg-gray-300 rounded-lg hover:bg-gray-400 transition"
                            >
                                Cancel
                            </button>
                            <button
                                onClick={handleCreate}
                                className="px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700 transition"
                            >
                                Create
                            </button>
                        </div>
                    </div>
                </div>
            )}

            {/* Existing File Browser Modal stays the same */}
            {fileBrowserVisible && (
                <div className="fixed inset-0 bg-black bg-opacity-40 flex items-center justify-center z-50">
                    <div className="bg-white border rounded-xl p-6 shadow-2xl w-[420px] h-[520px] overflow-auto relative">
                        <h3 className="font-bold mb-1 text-lg text-center">Select File to Load</h3>
                        <p className="text-center text-gray-600 text-sm mb-3">
                            Loading into <strong>Slot {activeSlot}</strong> on{" "}
                            <span className="font-medium">{deviceDisplayNames[selectedDevice]}</span>
                        </p>

                        {/* Breadcrumbs */}
                        <div className="flex flex-wrap items-center gap-1 mb-4 text-sm text-blue-600">
                            <span onClick={() => fetchFiles("")} className="cursor-pointer hover:underline">
                                Root
                            </span>
                            {currentPath
                                .split("/")
                                .filter((p) => p !== "")
                                .map((part, index, arr) => {
                                    const subPath = arr.slice(0, index + 1).join("/") + "/";
                                    return (
                                        <React.Fragment key={index}>
                                            <span className="text-gray-400">/</span>
                                            <span onClick={() => fetchFiles(subPath)} className="cursor-pointer hover:underline">
                                                {part.replace("/", "")}
                                            </span>
                                        </React.Fragment>
                                    );
                                })}
                        </div>

                        {loadingFiles ? (
                            <div className="flex justify-center items-center h-48">
                                <div className="animate-spin rounded-full h-10 w-10 border-b-2 border-blue-600"></div>
                            </div>
                        ) : (
                            <ul className="border border-gray-200 rounded-lg divide-y divide-gray-100">
                                {currentFiles.length === 0 ? (
                                    <li className="p-2 text-gray-500 italic text-center">(Empty directory)</li>
                                ) : (
                                    currentFiles.map((f, idx) => (
                                        <li
                                            key={idx}
                                            onClick={() => handleFileClick(f)}
                                            className={`cursor-pointer p-2 hover:bg-gray-100 ${f.endsWith("/") ? "font-semibold text-blue-700" : "text-gray-800"
                                                }`}
                                        >
                                            {f}
                                        </li>
                                    ))
                                )}
                            </ul>
                        )}

                        <button
                            onClick={() => setFileBrowserVisible(false)}
                            className="absolute bottom-4 left-1/2 -translate-x-1/2 px-4 py-2 bg-gray-300 rounded-lg hover:bg-gray-400 transition"
                        >
                            Close
                        </button>
                    </div>
                </div>
            )}
        </div>
    );
}
