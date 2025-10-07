import React from "react";

export default function SkylanderPanel({
  deviceData,
  onCreate,
  onLoad,
  onRemove,
}) {
  const orderedKeys = Object.keys(deviceData)
    .map(Number)
    .sort((a, b) => a - b);

  return (
    <table className="w-full text-left border-collapse">
      <thead>
        <tr className="border-b border-gray-300">
          <th className="p-2">Slot</th>
          <th className="p-2">Name</th>
          <th className="p-2">Actions</th>
        </tr>
      </thead>
      <tbody>
        {orderedKeys.map((key) => {
          const slot = deviceData[key];
          return (
            <tr key={key} className="border-b border-gray-200">
              <td className="p-2">{key}</td>
              <td className="p-2">{slot?.name}</td>
              <td className="p-2 space-x-2">
                <button onClick={() => onCreate(key)} className="px-3 py-1 bg-green-500 text-white rounded-xl text-sm">Create</button>
                <button onClick={() => onLoad(key)} className="px-3 py-1 bg-blue-500 text-white rounded-xl text-sm">Load</button>
                <button onClick={() => onRemove(key)} className="px-3 py-1 bg-red-500 text-white rounded-xl text-sm">Remove</button>
              </td>
            </tr>
          );
        })}
      </tbody>
    </table>
  );
}
