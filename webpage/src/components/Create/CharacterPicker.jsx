// src/components/Create/CharacterPicker.jsx
import React, { useMemo, useRef, useState, useEffect } from "react";

export default function CharacterPicker({
    items = [],                 // [{ id, variant, name }]
    onSelect,                   // (item) => void
    onClose,                    // () => void
    title = "Choose Character", // optional
    placeholder = "Search by name, id, or variant…",
}) {
    const [query, setQuery] = useState("");
    const [active, setActive] = useState(0);
    const listRef = useRef(null);
    const inputRef = useRef(null);

    useEffect(() => {
        inputRef.current?.focus();
    }, []);

    const filtered = useMemo(() => {
        const q = query.trim().toLowerCase();
        if (!q) return items;
        return items.filter((it) => {
            const idStr = String(it.id);
            const varDec = String(it.variant);
            const varHex = "0x" + it.variant.toString(16).padStart(4, "0");
            return (
                it.name.toLowerCase().includes(q) ||
                idStr.includes(q) ||
                varDec.includes(q) ||
                varHex.toLowerCase().includes(q)
            );
        });
    }, [items, query]);

    const handleKeyDown = (e) => {
        if (e.key === "ArrowDown") {
            e.preventDefault();
            setActive((a) => Math.min(a + 1, filtered.length - 1));
        } else if (e.key === "ArrowUp") {
            e.preventDefault();
            setActive((a) => Math.max(a - 1, 0));
        } else if (e.key === "Enter") {
            const item = filtered[active];
            if (item) onSelect(item);
        } else if (e.key === "Escape") {
            onClose?.();
        }
    };

    useEffect(() => {
        const el = listRef.current?.querySelector(`[data-idx="${active}"]`);
        el?.scrollIntoView({ block: "nearest" });
    }, [active]);

    return (
        <div className="fixed inset-0 bg-black/40 z-50 flex items-center justify-center">
            <div className="bg-white rounded-2xl shadow-2xl w-[720px] max-w-[95vw] max-h-[90vh] overflow-hidden flex flex-col">
                <div className="p-5 border-b">
                    <div className="flex items-center justify-between">
                        <h3 className="text-lg font-semibold">{title}</h3>
                        <button onClick={onClose} className="px-3 py-1 rounded-lg bg-gray-100 hover:bg-gray-200">Close</button>
                    </div>
                    <input
                        ref={inputRef}
                        value={query}
                        onChange={(e) => setQuery(e.target.value)}
                        onKeyDown={handleKeyDown}
                        placeholder={placeholder}
                        className="mt-3 w-full border rounded-xl px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-500"
                    />
                </div>

                <div ref={listRef} className="p-3 overflow-auto">
                    {filtered.length === 0 ? (
                        <div className="p-6 text-center text-gray-500">No matches.</div>
                    ) : (
                        <ul className="divide-y">
                            {filtered.map((it, idx) => (
                                <li
                                    key={`${it.id}-${it.variant}-${idx}`}
                                    data-idx={idx}
                                    onClick={() => onSelect(it)}
                                    onMouseEnter={() => setActive(idx)}
                                    className={`px-3 py-2 cursor-pointer rounded-lg ${idx === active ? "bg-blue-50" : "hover:bg-gray-50"
                                        }`}
                                >
                                    <div className="flex items-center justify-between">
                                        <div className="font-medium">{it.name}</div>
                                        <div className="text-xs text-gray-500 ml-3">
                                            ID: {it.id}
                                            {typeof it.variant === "number" ? (
                                                <>
                                                    &nbsp;&middot;&nbsp; VAR: 0x{it.variant.toString(16).toUpperCase().padStart(4, "0")}
                                                </>
                                            ) : null}
                                            &nbsp;&middot;&nbsp; 0x{Number(it.id).toString(16).toUpperCase()}
                                        </div>

                                    </div>
                                </li>
                            ))}
                        </ul>
                    )}
                </div>
            </div>
        </div>
    );
}
