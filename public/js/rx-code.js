class RxInlineCode extends HTMLSpanElement {
    constructor() {
        super();

        this.className =
            "font-mono bg-gray-100 text-pink-500 px-2 py-1 rounded-md";
    }
}

customElements.define("rx-inline-code", RxInlineCode, { extends: "span" });
