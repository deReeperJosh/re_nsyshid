import React, { useState } from "react";

/**
 * Dimensions Toypad layout:
 * - Row 1: slots 0,1,2
 * - Row 2: slots 3,4,5,6
 * Adds a Move button per slot that opens a modal with the same layout.
 * Clicking "Move Here" calls onMove(oldSlot, newSlot) provided by the parent.
 */
export default function DimensionsPanel({
  deviceData,
  onCreate,
  onLoad,
  onRemove,
  onMove, // (oldSlot, newSlot) => Promise|void
}) {
  const row1 = [0, 1, 2];
  const row2 = [3, 4, 5, 6];

  const [moveModalOpen, setMoveModalOpen] = useState(false);
  const [movingFrom, setMovingFrom] = useState(null);

  const openMoveModal = (fromIdx) => {
    setMovingFrom(fromIdx);
    setMoveModalOpen(true);
  };

  const closeMoveModal = () => {
    setMoveModalOpen(false);
    setMovingFrom(null);
  };

  const handleMoveHere = async (toIdx) => {
    try {
      await onMove?.(movingFrom, toIdx);
    } finally {
      closeMoveModal();
    }
  };

  const Cell = ({ idx }) => {
    const slot = deviceData[idx];
    const loaded = !!slot;

    return (
      <div className="border rounded-xl p-3 bg-white shadow-sm">
        <div className="flex items-center justify-between mb-2">
          <div className="text-sm font-semibold">Slot {idx}</div>
          <div className={`text-xs ${loaded ? "text-green-700" : "text-gray-500"}`}>
            {loaded ? "Loaded" : "Empty"}
          </div>
        </div>

        <div className="text-sm mb-3 min-h-[1.5rem]">
          {loaded ? (slot.name || "(unnamed)") : <span className="italic text-gray-500">—</span>}
        </div>

        <div className="flex flex-wrap gap-2">
          <button
            onClick={() => onCreate(idx)}
            className="px-3 py-1 bg-green-500 text-white rounded-xl text-sm hover:bg-green-600"
          >
            Create
          </button>
          <button
            onClick={() => onLoad(idx)}
            className="px-3 py-1 bg-blue-500 text-white rounded-xl text-sm hover:bg-blue-600"
          >
            Load
          </button>
          <button
            onClick={() => onRemove(idx)}
            className="px-3 py-1 bg-red-500 text-white rounded-xl text-sm hover:bg-red-600"
          >
            Remove
          </button>
          <button
            onClick={() => openMoveModal(idx)}
            className="px-3 py-1 bg-purple-600 text-white rounded-xl text-sm hover:bg-purple-700"
          >
            Move
          </button>
        </div>
      </div>
    );
  };

  const MoveGrid = () => (
    <div className="space-y-4">
      {/* Row 1: 3 cells */}
      <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
        {row1.map((i) => (
          <div key={i} className="border rounded-xl p-3 bg-white shadow-sm">
            <div className="text-sm font-semibold mb-2">Slot {i}</div>
            <button
              onClick={() => handleMoveHere(i)}
              className="w-full px-3 py-2 bg-purple-600 text-white rounded-xl text-sm hover:bg-purple-700 disabled:opacity-50"
              disabled={movingFrom === i}
            >
              {movingFrom === i ? "Current Position" : "Move Here"}
            </button>
          </div>
        ))}
      </div>

      {/* Row 2: 4 cells */}
      <div className="grid grid-cols-1 sm:grid-cols-4 gap-4">
        {row2.map((i) => (
          <div key={i} className="border rounded-xl p-3 bg-white shadow-sm">
            <div className="text-sm font-semibold mb-2">Slot {i}</div>
            <button
              onClick={() => handleMoveHere(i)}
              className="w-full px-3 py-2 bg-purple-600 text-white rounded-xl text-sm hover:bg-purple-700 disabled:opacity-50"
              disabled={movingFrom === i}
            >
              {movingFrom === i ? "Current Position" : "Move Here"}
            </button>
          </div>
        ))}
      </div>
    </div>
  );

  return (
    <>
      <div className="space-y-4">
        {/* Row 1: 3 cells */}
        <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
          {row1.map((i) => (
            <Cell idx={i} key={i} />
          ))}
        </div>

        {/* Row 2: 4 cells */}
        <div className="grid grid-cols-1 sm:grid-cols-4 gap-4">
          {row2.map((i) => (
            <Cell idx={i} key={i} />
          ))}
        </div>
      </div>

      {/* Move Modal */}
      {moveModalOpen && (
        <div className="fixed inset-0 bg-black/40 flex items-center justify-center z-50">
          <div
            role="dialog"
            aria-modal="true"
            className="bg-white rounded-2xl shadow-2xl w-[720px] max-w-[95vw] max-h-[90vh] overflow-hidden flex flex-col"
          >
            <div className="p-6 border-b">
              <h3 className="text-lg font-bold text-center">Move Character</h3>
              <p className="text-center text-sm text-gray-600 mt-1">
                Moving from <strong>Slot {movingFrom}</strong>
              </p>
            </div>

            <div className="p-6 overflow-auto">
              <MoveGrid />
            </div>

            <div className="p-4 border-t bg-gray-50">
              <button
                onClick={closeMoveModal}
                className="w-full px-4 py-2 bg-gray-300 rounded-lg hover:bg-gray-400 transition"
              >
                Cancel
              </button>
            </div>
          </div>
        </div>
      )}
    </>
  );
}
