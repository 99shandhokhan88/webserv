#!/usr/bin/env python3
"""
Script per analizzare le funzioni inutilizzate nel webserver C++
Analizza tutti i file .cpp e .hpp per trovare funzioni definite ma mai chiamate
"""

import os
import re
import glob
from collections import defaultdict

def find_cpp_files(directory):
    """Trova tutti i file .cpp e .hpp nella directory"""
    cpp_files = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(('.cpp', '.hpp')):
                cpp_files.append(os.path.join(root, file))
    return cpp_files

def extract_function_definitions(file_path):
    """Estrae tutte le definizioni di funzioni da un file"""
    functions = []
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # Pattern per funzioni membro di classe (Class::function)
        class_method_pattern = r'(\w+)::(\w+)\s*\([^)]*\)\s*(?:const)?\s*{'
        
        # Pattern per funzioni globali
        global_function_pattern = r'^(?:static\s+)?(?:inline\s+)?(?:\w+\s+)*(\w+)\s*\([^)]*\)\s*{'
        
        # Pattern per dichiarazioni di funzioni negli header
        declaration_pattern = r'(?:static\s+)?(?:virtual\s+)?(?:\w+\s+)*(\w+)\s*\([^)]*\)\s*(?:const)?\s*;'
        
        # Trova metodi di classe
        for match in re.finditer(class_method_pattern, content, re.MULTILINE):
            class_name = match.group(1)
            method_name = match.group(2)
            functions.append({
                'name': f"{class_name}::{method_name}",
                'type': 'method',
                'file': file_path,
                'line': content[:match.start()].count('\n') + 1
            })
        
        # Trova funzioni globali (solo nei .cpp)
        if file_path.endswith('.cpp'):
            for match in re.finditer(global_function_pattern, content, re.MULTILINE):
                func_name = match.group(1)
                # Esclude costruttori, distruttori e operatori
                if not re.match(r'(if|for|while|switch|catch|~\w+|\w+::\w+)', func_name):
                    functions.append({
                        'name': func_name,
                        'type': 'global',
                        'file': file_path,
                        'line': content[:match.start()].count('\n') + 1
                    })
        
        # Trova dichiarazioni negli header
        if file_path.endswith('.hpp'):
            for match in re.finditer(declaration_pattern, content, re.MULTILINE):
                func_name = match.group(1)
                # Esclude costruttori, distruttori e parole chiave
                if not re.match(r'(if|for|while|switch|catch|~\w+|class|struct|enum)', func_name):
                    functions.append({
                        'name': func_name,
                        'type': 'declaration',
                        'file': file_path,
                        'line': content[:match.start()].count('\n') + 1
                    })
                    
    except Exception as e:
        print(f"Errore leggendo {file_path}: {e}")
    
    return functions

def find_function_calls(file_path, function_name):
    """Cerca le chiamate a una funzione specifica in un file"""
    calls = []
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # Pattern per chiamate di funzione
        # Gestisce sia chiamate dirette che tramite puntatori/oggetti
        simple_name = function_name.split('::')[-1]  # Prende solo il nome senza classe
        
        patterns = [
            rf'\b{re.escape(function_name)}\s*\(',  # Chiamata completa Class::method(
            rf'\b{re.escape(simple_name)}\s*\(',    # Chiamata semplice method(
            rf'->\s*{re.escape(simple_name)}\s*\(', # Chiamata tramite puntatore ->method(
            rf'\.\s*{re.escape(simple_name)}\s*\(',  # Chiamata tramite oggetto .method(
        ]
        
        for pattern in patterns:
            for match in re.finditer(pattern, content):
                line_num = content[:match.start()].count('\n') + 1
                calls.append({
                    'file': file_path,
                    'line': line_num,
                    'context': content.split('\n')[line_num-1].strip()
                })
                
    except Exception as e:
        print(f"Errore cercando chiamate in {file_path}: {e}")
    
    return calls

def analyze_functions():
    """Analizza tutte le funzioni e trova quelle inutilizzate"""
    
    # Trova tutti i file C++
    cpp_files = find_cpp_files('srcs')
    print(f"Trovati {len(cpp_files)} file C++")
    
    # Estrai tutte le definizioni di funzioni
    all_functions = []
    for file_path in cpp_files:
        functions = extract_function_definitions(file_path)
        all_functions.extend(functions)
    
    print(f"Trovate {len(all_functions)} definizioni di funzioni")
    
    # Analizza l'utilizzo di ogni funzione
    unused_functions = []
    used_functions = []
    
    for func in all_functions:
        func_name = func['name']
        total_calls = 0
        call_locations = []
        
        # Cerca chiamate in tutti i file
        for file_path in cpp_files:
            calls = find_function_calls(file_path, func_name)
            total_calls += len(calls)
            call_locations.extend(calls)
        
        # Filtra le chiamate che sono nella stessa riga della definizione
        actual_calls = []
        for call in call_locations:
            if not (call['file'] == func['file'] and abs(call['line'] - func['line']) <= 2):
                actual_calls.append(call)
        
        if len(actual_calls) == 0:
            unused_functions.append({
                'function': func,
                'calls': 0,
                'locations': []
            })
        else:
            used_functions.append({
                'function': func,
                'calls': len(actual_calls),
                'locations': actual_calls
            })
    
    return unused_functions, used_functions

def main():
    print("=== ANALISI FUNZIONI INUTILIZZATE WEBSERVER ===\n")
    
    unused, used = analyze_functions()
    
    print(f"📊 STATISTICHE:")
    print(f"   - Funzioni totali: {len(unused) + len(used)}")
    print(f"   - Funzioni utilizzate: {len(used)}")
    print(f"   - Funzioni NON utilizzate: {len(unused)}")
    print(f"   - Percentuale inutilizzate: {len(unused)/(len(unused)+len(used))*100:.1f}%\n")
    
    if unused:
        print("🚨 FUNZIONI POTENZIALMENTE INUTILIZZATE:")
        print("=" * 60)
        
        # Raggruppa per file
        by_file = defaultdict(list)
        for item in unused:
            by_file[item['function']['file']].append(item)
        
        for file_path, functions in by_file.items():
            print(f"\n📁 {file_path}:")
            for item in functions:
                func = item['function']
                print(f"   ❌ {func['name']} (linea {func['line']}) - {func['type']}")
    
    # Mostra alcune funzioni più utilizzate
    if used:
        print(f"\n✅ TOP 10 FUNZIONI PIÙ UTILIZZATE:")
        print("=" * 50)
        used_sorted = sorted(used, key=lambda x: x['calls'], reverse=True)[:10]
        for item in used_sorted:
            func = item['function']
            print(f"   🔥 {func['name']} - {item['calls']} chiamate")
    
    # Salva report dettagliato
    with open('function_analysis_report.txt', 'w') as f:
        f.write("REPORT ANALISI FUNZIONI WEBSERVER\n")
        f.write("=" * 50 + "\n\n")
        
        f.write("FUNZIONI INUTILIZZATE:\n")
        f.write("-" * 30 + "\n")
        for item in unused:
            func = item['function']
            f.write(f"{func['name']} - {func['file']}:{func['line']} ({func['type']})\n")
        
        f.write(f"\nFUNZIONI UTILIZZATE:\n")
        f.write("-" * 30 + "\n")
        for item in used:
            func = item['function']
            f.write(f"{func['name']} - {item['calls']} chiamate\n")
            for call in item['locations'][:3]:  # Prime 3 chiamate
                f.write(f"  -> {call['file']}:{call['line']}\n")
            if len(item['locations']) > 3:
                f.write(f"  -> ... e altre {len(item['locations'])-3} chiamate\n")
    
    print(f"\n📄 Report dettagliato salvato in: function_analysis_report.txt")

if __name__ == "__main__":
    main() 