import React from "react";

const LABELS = {
  0: "Play Set/Power Disc",
  1: "Power Disc Two",
  2: "Power Disc Three",
  3: "Player One",
  4: "Ability One (P1)",
  5: "Ability Two (P1)",
  6: "Player Two",
  7: "Ability One (P2)",
  8: "Ability Two (P2)",
};

export default function InfinityPanel({
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
          <th className="p-2">Position</th>
          <th className="p-2">Actions</th>
        </tr>
      </thead>
      <tbody>
        {orderedKeys.map((key) => (
          <tr key={key} className="border-b border-gray-200">
            <td className="p-2">{key}</td>
            <td className="p-2">{LABELS[key] ?? `Slot ${key}`}</td>
            <td className="p-2 space-x-2">
              <button onClick={() => onCreate(key)} className="px-3 py-1 bg-green-500 text-white rounded-xl text-sm">Create</button>
              <button onClick={() => onLoad(key)} className="px-3 py-1 bg-blue-500 text-white rounded-xl text-sm">Load</button>
              <button onClick={() => onRemove(key)} className="px-3 py-1 bg-red-500 text-white rounded-xl text-sm">Remove</button>
            </td>
          </tr>
        ))}
      </tbody>
    </table>
  );
}
