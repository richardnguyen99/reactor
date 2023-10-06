class RxLink extends HTMLAnchorElement {
    static linkIcon = `
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" width="16" height="16" class="inline-block w-4 h-4 align-middle mr-1">
        <path d="M2.75 3.5a.25.25 0 0 0-.25.25v7.5c0 .138.112.25.25.25h2a.75.75 0 0 1 .75.75v2.19l2.72-2.72a.749.749 0 0 1 .53-.22h4.5a.25.25 0 0 0 .25-.25v-2.5a.75.75 0 0 1 1.5 0v2.5A1.75 1.75 0 0 1 13.25 13H9.06l-2.573 2.573A1.458 1.458 0 0 1 4 14.543V13H2.75A1.75 1.75 0 0 1 1 11.25v-7.5C1 2.784 1.784 2 2.75 2h5.5a.75.75 0 0 1 0 1.5ZM16 1.25v4.146a.25.25 0 0 1-.427.177L14.03 4.03l-3.75 3.75a.749.749 0 0 1-1.275-.326.749.749 0 0 1 .215-.734l3.75-3.75-1.543-1.543A.25.25 0 0 1 11.604 1h4.146a.25.25 0 0 1 .25.25Z"></path>
    </svg>
    `;

    constructor() {
        super();

        this.className =
            "inline-flex items-center gap-1 " +
            "fill-sky-500 text-sky-500 hover:text-sky-600 hover:underline " +
            "cursor-pointer";

        // Add svg icon to the custom link element after the text
        this.insertAdjacentHTML("beforeend", RxLink.linkIcon);

        // Add event listener to the custom link element
        this.addEventListener("click", this.onClick.bind(this));
    }

    onClick(event) {}
}

customElements.define("rx-link", RxLink, { extends: "a" });
