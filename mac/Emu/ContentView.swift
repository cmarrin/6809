//
//  ContentView.swift
//  Emu
//
//  Created by Chris Marrin on 5/18/24.
//

import SwiftUI

struct ContentView: View {
    @State private var consoleText = "BOSS> "

    var body: some View {
        HStack {
            TextEditor(text: $consoleText)
                .padding()
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
